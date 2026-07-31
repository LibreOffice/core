using System.Xml.Linq;
using Paperless.Containers;
using Paperless.Containers.Ooxml;
using Paperless.Core;
using Paperless.Core.Diagnostics;
using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.Ooxml;

namespace Paperless.Presentations.Ooxml;

/// <summary>
/// An open PPTX package: its presentation part, its slides in presentation order, and the
/// layout, master and notes parts each slide reaches.
/// </summary>
/// <remarks>
/// <para>
/// Everything is located by <em>relationship</em>. <c>ppt/presentation.xml</c>,
/// <c>ppt/slides/slide1.xml</c> and the rest are conventions PowerPoint happens to follow, not
/// rules: the specification permits any part name, and the slide <em>order</em> is
/// <c>p:sldIdLst</c>'s order of <c>r:id</c>s, which is not the parts' alphabetical order and
/// diverges from it as soon as a deck has ten slides or a slide is moved. Conventional names
/// survive only as a last-ditch fallback for packages that lose their root relationship.
/// </para>
/// <para>
/// The chain is slide → layout → master, each hop a relationship <em>from the part below</em>:
/// a slide names its layout, and the layout — not the slide — names the master. A deck with
/// several masters has no other way to say which slide belongs to which
/// (<c>oox/source/ppt/presentationfragmenthandler.cxx:589-600</c>).
/// </para>
/// </remarks>
internal sealed class PptxFile : IDisposable
{
    private const string RelationshipBase =
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/";

    private readonly OpcPackage _package;
    private readonly List<Diagnostic> _diagnostics = [];
    private readonly Dictionary<string, XElement?> _parts = new(StringComparer.Ordinal);
    private readonly Dictionary<string, Dictionary<string, OpcXml.Relationship>> _relationships =
        new(StringComparer.Ordinal);

    private PptxFile(OpcPackage package, IPackagePart mainPart, XElement presentation)
    {
        _package = package;
        MainPartName = mainPart.Name;
        Presentation = presentation;
        _diagnostics.AddRange(package.Diagnostics);

        SlideSize = ReadSlideSize(Ppt.Child(presentation, "sldSz"));
        DefaultTextStyle = Ppt.Child(presentation, "defaultTextStyle");
        Slides = [.. ReadSlideList()];
    }

    /// <summary>The presentation part's name, for diagnostics and relationship resolution.</summary>
    public string MainPartName { get; }

    /// <summary>The <c>p:presentation</c> root.</summary>
    public XElement Presentation { get; }

    /// <summary>
    /// The slide size from <c>p:sldSz</c>, or a zero size when the deck states none.
    /// </summary>
    /// <remarks>
    /// In EMUs directly: PresentationML states slide dimensions in EMUs, so unlike almost every
    /// other measurement in an office format this one needs no conversion.
    /// </remarks>
    public DocSize SlideSize { get; }

    /// <summary>
    /// The presentation-wide <c>p:defaultTextStyle</c>, the last stop in the list-style chain.
    /// </summary>
    public XElement? DefaultTextStyle { get; }

    /// <summary>The deck's slides, in presentation order.</summary>
    public IReadOnlyList<PptxSlide> Slides { get; }

    /// <summary>The package, for reaching image and media parts.</summary>
    public IPackage Package => _package;

    /// <summary>Problems found while opening the deck.</summary>
    public IReadOnlyList<Diagnostic> Diagnostics => _diagnostics;

    /// <summary>Opens a PPTX over a seekable stream.</summary>
    /// <param name="stream">The package bytes.</param>
    /// <param name="leaveOpen">When true, disposing this does not dispose the stream.</param>
    /// <exception cref="MalformedDocumentException">
    /// There is no readable presentation part, so there is nothing to read.
    /// </exception>
    public static PptxFile Open(Stream stream, bool leaveOpen = false)
    {
        ArgumentNullException.ThrowIfNull(stream);

        OpcPackage package = OpcPackage.Open(stream, leaveOpen);
        try
        {
            IPackagePart? main = package.GetMainDocumentPart() ?? package.GetPart("ppt/presentation.xml");
            if (main is null)
            {
                throw new MalformedDocumentException(
                    "The OOXML package has no presentation part: neither the officeDocument "
                    + "relationship nor the conventional 'ppt/presentation.xml' resolves.");
            }

            XElement? root;
            string? error;
            using (Stream content = main.Open()) root = OoxmlXml.TryLoad(content, out error);

            if (root is null)
            {
                throw new MalformedDocumentException(
                    $"The presentation part '{main.Name}' is not readable XML"
                    + (error is null ? "." : $": {error}"));
            }

            return new PptxFile(package, main, root);
        }
        catch
        {
            package.Dispose();
            throw;
        }
    }

    /// <summary>
    /// The relationship a part declares under an id, or null.
    /// </summary>
    public OpcXml.Relationship? Relationship(string partName, string? relationshipId)
    {
        if (relationshipId is null) return null;
        return RelationshipsOf(partName).TryGetValue(relationshipId, out OpcXml.Relationship found)
            ? found
            : null;
    }

    /// <summary>
    /// The first relationship of a type a part declares, or null.
    /// </summary>
    /// <remarks>
    /// First rather than only: a slide declares exactly one <c>slideLayout</c> and at most one
    /// <c>notesSlide</c>, but a malformed producer may repeat one, and taking the first is what
    /// LibreOffice's <c>getFragmentPathFromFirstTypeFromOfficeDoc</c> does.
    /// </remarks>
    public string? TargetOfType(string partName, string relationshipSuffix)
    {
        foreach (OpcXml.Relationship relationship in RelationshipsOf(partName).Values)
        {
            if (!relationship.IsExternal
                && string.Equals(relationship.Type, RelationshipBase + relationshipSuffix,
                                 StringComparison.Ordinal))
                return relationship.Target;
        }
        return null;
    }

    /// <summary>
    /// Loads and caches a part's root element, or null when it is absent or unreadable.
    /// </summary>
    /// <remarks>
    /// Cached because a layout is shared by every slide that uses it and a master by every
    /// layout: a fifty-slide deck on one layout would otherwise parse the same two parts fifty
    /// times, and the placeholder lookup walks them for every placeholder on every slide.
    /// </remarks>
    public XElement? Load(string? partName)
    {
        if (partName is null) return null;
        if (_parts.TryGetValue(partName, out XElement? cached)) return cached;

        XElement? root = null;
        if (_package.GetPart(partName) is { } part)
        {
            using Stream content = part.Open();
            root = OoxmlXml.TryLoad(content, out string? error);
            if (root is null && error is not null)
            {
                _diagnostics.Add(new Diagnostic(
                    DiagnosticSeverity.Error, "PL2210",
                    $"The part '{partName}' is malformed and has been skipped: {error.TrimEnd('.')}.",
                    new DiagnosticLocation(partName)));
            }
        }

        _parts[partName] = root;
        return root;
    }

    /// <summary>Records a problem found while reading content.</summary>
    public void Report(Diagnostic diagnostic) => _diagnostics.Add(diagnostic);

    /// <inheritdoc/>
    public void Dispose() => _package.Dispose();

    private Dictionary<string, OpcXml.Relationship> RelationshipsOf(string partName)
    {
        if (_relationships.TryGetValue(partName, out Dictionary<string, OpcXml.Relationship>? cached))
            return cached;

        Dictionary<string, OpcXml.Relationship> map = new(StringComparer.Ordinal);
        foreach (OpcXml.Relationship relationship in _package.GetRelationships(partName))
            map[relationship.Id] = relationship;

        _relationships[partName] = map;
        return map;
    }

    private static DocSize ReadSlideSize(XElement? size)
        => new(
            Length.FromEmu(long.TryParse(Ppt.Attribute(size, "cx"), out long cx) ? cx : 0),
            Length.FromEmu(long.TryParse(Ppt.Attribute(size, "cy"), out long cy) ? cy : 0));

    /// <summary>
    /// Resolves <c>p:sldIdLst</c> into slide records, in the order it states.
    /// </summary>
    /// <remarks>
    /// A <c>p:sldId</c> whose <c>r:id</c> resolves to nothing is reported and skipped rather
    /// than failing the read: the rest of the deck is still perfectly readable, and a deck that
    /// has lost one slide part is a real thing that happens to files edited by tools.
    /// </remarks>
    private IEnumerable<PptxSlide> ReadSlideList()
    {
        int index = 0;
        foreach (XElement id in Ppt.Children(Ppt.Child(Presentation, "sldIdLst"), "sldId"))
        {
            string? relationshipId = Ppt.RelationshipId(id);
            OpcXml.Relationship? relationship = Relationship(MainPartName, relationshipId);

            if (relationship is not { IsExternal: false } resolved || Load(resolved.Target) is not { } root)
            {
                _diagnostics.Add(new Diagnostic(
                    DiagnosticSeverity.Warning, "PL2211",
                    $"Slide {index + 1} names relationship '{relationshipId}', which does not "
                    + "resolve to a readable part; the slide has been skipped.",
                    new DiagnosticLocation(MainPartName)));
                index++;
                continue;
            }

            yield return CreateSlide(index++, resolved.Target, root);
        }
    }

    private PptxSlide CreateSlide(int index, string partName, XElement root)
    {
        string? layoutPart = TargetOfType(partName, "slideLayout");
        XElement? layout = Load(layoutPart);

        // The master hangs off the layout, not the slide. A slide that names no layout — which
        // the schema forbids but producers manage anyway — simply has no inherited placeholders.
        string? masterPart = layoutPart is null ? null : TargetOfType(layoutPart, "slideMaster");
        XElement? master = Load(masterPart);

        string? notesPart = TargetOfType(partName, "notesSlide");

        return new PptxSlide(
            index,
            partName,
            root,
            layoutPart,
            layout,
            masterPart,
            master,
            notesPart,
            Load(notesPart));
    }
}

/// <summary>
/// One slide, with the parts it inherits from already resolved.
/// </summary>
/// <param name="Index">The zero-based position in <c>p:sldIdLst</c>.</param>
/// <param name="PartName">The slide part's name.</param>
/// <param name="Root">The <c>p:sld</c> root.</param>
/// <param name="LayoutPartName">The layout part's name, or null.</param>
/// <param name="Layout">The <c>p:sldLayout</c> root, or null.</param>
/// <param name="MasterPartName">The master part's name, or null.</param>
/// <param name="Master">The <c>p:sldMaster</c> root, or null.</param>
/// <param name="NotesPartName">The notes-slide part's name, or null.</param>
/// <param name="Notes">The <c>p:notes</c> root, or null.</param>
internal sealed record PptxSlide(
    int Index,
    string PartName,
    XElement Root,
    string? LayoutPartName,
    XElement? Layout,
    string? MasterPartName,
    XElement? Master,
    string? NotesPartName,
    XElement? Notes)
{
    /// <summary>
    /// The name the deck gives this slide, or null.
    /// </summary>
    /// <remarks>
    /// <c>p:cSld/@name</c>, which is optional and which LibreOffice's own PPTX export does not
    /// write at all — so a deck round-tripped through Impress has unnamed slides, and reporting
    /// a manufactured "Slide 3" instead of null would claim the file says something it does not.
    /// </remarks>
    public string? Name
    {
        get
        {
            string? name = Ppt.Attribute(Ppt.Child(Root, "cSld"), "name");
            return string.IsNullOrEmpty(name) ? null : name;
        }
    }

    /// <summary>
    /// True when the slide is skipped during a show.
    /// </summary>
    /// <remarks>
    /// <c>p:sld/@show</c>, absent meaning shown
    /// (<c>oox/source/ppt/slidefragmenthandler.cxx:100</c>). The slide's content is extracted
    /// either way — a hidden slide is still text the deck contains — and flagged so a caller
    /// reproducing what a viewer shows can drop it.
    /// </remarks>
    public bool IsHidden => !Ppt.Flag(Root, "show", whenAbsent: true);

    /// <summary>The slide's <c>p:spTree</c>, or null when the part is malformed.</summary>
    public XElement? ShapeTree => Ppt.Child(Ppt.Child(Root, "cSld"), "spTree");
}
