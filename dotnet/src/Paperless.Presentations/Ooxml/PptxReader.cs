using System.Xml.Linq;
using Paperless.Containers.Ooxml;
using Paperless.Core.Diagnostics;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;
using Paperless.Core.Geometry;
using Paperless.Ooxml;

namespace Paperless.Presentations.Ooxml;

/// <summary>
/// Reads OOXML presentations: <c>pptx</c>, <c>pptm</c>, the <c>potx</c>/<c>potm</c> templates and
/// the <c>ppsx</c>/<c>ppsm</c> shows.
/// </summary>
/// <remarks>
/// <para>
/// The six formats differ only in their content type. A template is a deck whose parts are named
/// the same and read the same; a show is a deck with a flag telling PowerPoint to open it in
/// presentation mode. The macro-enabled variants are read as data like any other, and nothing is
/// ever executed.
/// </para>
/// <para>
/// Each slide becomes a <see cref="SectionKind.Slide"/> section, with its speaker notes following
/// as a <see cref="SectionKind.SlideNotes"/> section so the two stay adjacent and separately
/// identifiable — the same shape the ODP path produces, so a caller indexing a mixed corpus sees
/// one structure rather than two.
/// </para>
/// </remarks>
public static class PptxReader
{
    /// <summary>Reads a deck, leaving the source's stream for the caller to dispose.</summary>
    /// <param name="source">The deck to read.</param>
    /// <param name="format">The identified format, recorded on the result.</param>
    public static PptxDocument Read(DocumentSource source, DocumentFormat format)
    {
        ArgumentNullException.ThrowIfNull(source);

        PptxFile file = PptxFile.Open(source.Stream, leaveOpen: true);
        try
        {
            ContentDocument content = new()
            {
                Metadata = OoxmlMetadata.Read((OpcPackage)file.Package, DocumentFamily.Presentation),
            };

            if (file.Slides.Count == 0)
            {
                // A deck with no p:sldIdLst is readable — its metadata and masters are intact —
                // so this is a diagnostic rather than an exception. A template legitimately has
                // no slides at all, which is why the severity is not an error.
                file.Report(new Diagnostic(
                    DiagnosticSeverity.Information, "PL2212",
                    "The presentation lists no slides, so it has no slide content to extract.",
                    new DiagnosticLocation(file.MainPartName)));
            }

            foreach (PptxSlide slide in file.Slides) ReadSlide(file, slide, content);

            return new PptxDocument(format, file, content);
        }
        catch
        {
            file.Dispose();
            throw;
        }
    }

    private static void ReadSlide(PptxFile file, PptxSlide slide, ContentDocument content)
    {
        ContentSection section = new()
        {
            Kind = SectionKind.Slide,
            Index = slide.Index,
            Name = slide.Name,
            IsHidden = slide.IsHidden,
        };

        if (slide.ShapeTree is { } tree)
        {
            PptxTextStyles styles = new(
                slide.Layout, slide.Master, file.DefaultTextStyle, isNotesPage: false);
            new PptxShapeReader(file, slide.PartName, styles).Read(tree, section);
        }
        else
        {
            file.Report(new Diagnostic(
                DiagnosticSeverity.Warning, "PL2213",
                $"Slide {slide.Index + 1} has no p:cSld/p:spTree, so it contributes no content.",
                new DiagnosticLocation(slide.PartName)));
        }

        content.Children.Add(section);

        if (ReadNotes(file, slide) is { } notes) content.Children.Add(notes);
    }

    /// <summary>
    /// Reads a slide's notes page, or returns null when it has nothing to say.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The notes slide is a part of its own, reached by relationship from the slide — never by
    /// assuming <c>notesSlide{n}.xml</c> matches <c>slide{n}.xml</c>, which they stop doing as
    /// soon as one slide in the middle of a deck has notes and its neighbours do not.
    /// </para>
    /// <para>
    /// It inherits from the <em>notes</em> master, not the slide master, and its body
    /// placeholder resolves against <c>p:notesStyle</c> rather than <c>p:bodyStyle</c>
    /// (<c>oox/source/ppt/pptshape.cxx:126-136</c>). Getting that wrong bullets every line of
    /// every speaker note.
    /// </para>
    /// <para>
    /// Null when the page holds no text: PowerPoint and Impress both write a notes part for
    /// slides that have no notes, carrying only a slide-image placeholder and an empty body, and
    /// reporting those would attach an empty notes section to every slide in every deck.
    /// </para>
    /// </remarks>
    private static ContentSection? ReadNotes(PptxFile file, PptxSlide slide)
    {
        if (slide.Notes is null || slide.NotesPartName is null) return null;

        XElement? tree = Ppt.Child(Ppt.Child(slide.Notes, "cSld"), "spTree");
        if (tree is null) return null;

        XElement? notesMaster = file.Load(file.TargetOfType(slide.NotesPartName, "notesMaster"));

        ContentSection section = new()
        {
            Kind = SectionKind.SlideNotes,
            Index = slide.Index,
            Name = slide.Name,
        };

        PptxTextStyles styles = new(
            layout: null, notesMaster, file.DefaultTextStyle, isNotesPage: true);
        new PptxShapeReader(file, slide.NotesPartName, styles).Read(Filtered(tree), section);

        return section.GetText().Trim().Length > 0 ? section : null;
    }

    /// <summary>
    /// A notes page's shape tree without its slide-image placeholder.
    /// </summary>
    /// <remarks>
    /// <c>&lt;p:ph type="sldImg"/&gt;</c> is a live preview of the slide the notes belong to, not
    /// an embedded picture. Reporting it as a graphic would claim every notes page in every deck
    /// contains an image, and its content — the slide — has already been extracted next to it.
    /// </remarks>
    private static XElement Filtered(XElement tree)
    {
        XElement copy = new(tree.Name, tree.Attributes());
        foreach (XElement shape in tree.Elements())
        {
            if (PptxPlaceholder.Element(shape) is { } placeholder
                && Ppt.Attribute(placeholder, "type") == "sldImg")
                continue;
            copy.Add(shape);
        }
        return copy;
    }
}

/// <summary>An OOXML presentation that has been read.</summary>
public sealed class PptxDocument : IDocument
{
    private readonly PptxFile _file;

    internal PptxDocument(DocumentFormat format, PptxFile file, ContentDocument content)
    {
        Format = format;
        _file = file;
        Content = content;
    }

    /// <inheritdoc/>
    public DocumentFormat Format { get; }

    /// <inheritdoc/>
    public DocumentFamily Family => DocumentFamily.Presentation;

    /// <inheritdoc/>
    public DocumentMetadata Metadata => Content.Metadata;

    /// <inheritdoc/>
    public ContentDocument Content { get; }

    /// <inheritdoc/>
    public IReadOnlyList<Diagnostic> Diagnostics => _file.Diagnostics;

    /// <summary>
    /// The slide size from <c>p:sldSz</c>.
    /// </summary>
    /// <remarks>
    /// Reported even though extraction does not use it, because it is the one piece of
    /// presentation geometry a caller reliably wants without laying anything out — deciding
    /// whether a deck is 4:3 or 16:9 needs no fonts and no rasteriser.
    /// </remarks>
    public DocSize SlideSize => _file.SlideSize;

    /// <inheritdoc/>
    public void Dispose() => _file.Dispose();
}
