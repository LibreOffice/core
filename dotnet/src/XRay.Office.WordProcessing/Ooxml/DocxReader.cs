using System.Xml.Linq;
using XRay.Office.Containers.Ooxml;
using XRay.Office.Core.Diagnostics;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Extraction;
using XRay.Office.Core.Formats;
using XRay.Office.Ooxml;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Model;

namespace XRay.Office.WordProcessing.Ooxml;

/// <summary>
/// Reads OOXML word-processing documents: <c>docx</c>, <c>docm</c>, and the <c>dotx</c> and
/// <c>dotm</c> templates.
/// </summary>
/// <remarks>
/// <para>
/// The macro-enabled variants are read as data like any other. A macro's presence is reported
/// through <c>FormatInfo.CanCarryMacros</c>; nothing is ever executed.
/// </para>
/// <para>
/// The body is one flow and becomes one <see cref="SectionKind.Body"/> section. Headers,
/// footers, footnotes, endnotes, comments and text-box flows are separate flows and become
/// their own sections — LibreOffice's own text export drops all of them, so extracting them is a
/// deliberate improvement rather than a difference to reconcile.
/// </para>
/// </remarks>
public static class DocxReader
{
    private const string HeaderRelationship =
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/header";
    private const string FooterRelationship =
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/footer";

    /// <summary>Reads a document, leaving the source's stream for the caller to dispose.</summary>
    /// <param name="source">The document to read.</param>
    /// <param name="format">The identified format, recorded on the result.</param>
    public static OoxmlWordDocument Read(DocumentSource source, DocumentFormat format)
    {
        ArgumentNullException.ThrowIfNull(source);

        DocxFile file = DocxFile.Open(source.Stream, leaveOpen: true);
        try
        {
            List<Diagnostic> diagnostics = [.. file.Diagnostics];
            List<WritingSection> sections = [];
            WritingMarks marks = WritingMarks.Empty;
            ContentDocument content = new()
            {
                Metadata = OoxmlMetadata.Read(
                    (OpcPackage)file.Package, DocumentFamily.WordProcessing),
            };

            XElement? body = file.Body;
            if (body is null)
            {
                diagnostics.Add(new Diagnostic(
                    DiagnosticSeverity.Error, "PL2130",
                    "The document has no w:body, so it has no content to extract."));
            }
            else
            {
                DocxContentReader reader = new(file, diagnostics);

                ContentSection section = new() { Kind = SectionKind.Body, Index = 0 };
                reader.ReadBlocks(body, section);
                content.Children.Add(section);

                // Notes and comments land immediately after the body they belong to rather than
                // at the very end, so their position stays meaningful.
                foreach (ContentNode node in reader.TakeHoisted()) content.Children.Add(node);

                ReadHeadersAndFooters(file, body, reader, content);
                sections = ReadSections(file, body);
                marks = reader.Marks;
            }

            return new OoxmlWordDocument(
                format, file, content, diagnostics, sections, marks, source.FileName);
        }
        catch
        {
            file.Dispose();
            throw;
        }
    }

    /// <summary>
    /// Reads every section's page geometry, in document order.
    /// </summary>
    /// <remarks>
    /// A DOCX states a section's properties at its <em>end</em>: each <c>w:sectPr</c> inside a
    /// paragraph's properties closes the section that paragraph finishes, and the one directly under
    /// <c>w:body</c> closes the last. So the enumeration is already in document order, and a document
    /// with no <c>w:sectPr</c> at all still has one section — of default geometry, not of none.
    /// </remarks>
    private static List<WritingSection> ReadSections(DocxFile file, XElement body)
    {
        List<WritingSection> sections =
        [
            .. DocxContentReader.SectionProperties(body)
                .Select(properties => DocxPageGeometry.Read(properties, file.Settings)),
        ];

        if (sections.Count == 0) sections.Add(DocxPageGeometry.Read(null, file.Settings));

        PromoteContinuousAcrossOrientation(sections);

        // Under `w:compat`, not directly under `w:settings` — `SettingsTable` reads it as
        // `LN_CT_Compat_noColumnBalance` (`dmapper/SettingsTable.cxx`:418). Looking for it one level too
        // high finds nothing and silently balances a document that asked not to be.
        SectionColumnBalance.Apply(
            sections,
            Word.IsOn(Word.Child(Word.Child(file.Settings, "compat"), "noColumnBalance")));

        return sections;
    }

    /// <summary>
    /// Turns a <c>continuous</c> section break into a <c>nextPage</c> one when the section's
    /// <c>w:orient</c> flag differs from the previous section's.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A sheet has one orientation, so a section that wants the other one cannot share a page with the
    /// section above it — LibreOffice says so in as many words and does exactly this, in
    /// <c>SectionPropertyMap::CloseSectionGroup</c>
    /// (<c>sw/source/writerfilter/dmapper/PropertyMap.cxx</c>:1661-1678): "if page orientation differs
    /// from previous section, it can't be treated as continuous", and the break type becomes
    /// <c>nextPage</c>.
    /// </para>
    /// <para>
    /// <strong>The comparison is on the flag and on nothing else</strong>, which is the part worth
    /// measuring rather than assuming. The property compared is <c>PROP_IS_LANDSCAPE</c>, written once
    /// from <c>w:orient</c> alone (<c>DomainMapper.cxx</c>:2859 and :830) after being reset to false, so
    /// the stated width and height never enter it. Swept on the installed 26.2.4.2, one section holding a
    /// page and a line so a promoted break shows as an extra page: a continuous section 720 twips wider,
    /// 720 twips taller, one twip wider, carrying <c>w:orient="portrait"</c> where the previous section
    /// carried none, or an inch of extra top margin, all stay on the page. A continuous section stating
    /// <c>w:orient="landscape"</c> takes a new one. The corner that settles it: a first section stating
    /// <c>15840 × 12240</c> with <em>no</em> <c>w:orient</c> — a physically landscape sheet — followed by
    /// a continuous portrait-shaped section does <strong>not</strong> break, because neither carries the
    /// flag; state <c>w:orient="landscape"</c> on the second and it does. Eight authored variants, six of
    /// them controls.
    /// </para>
    /// <para>
    /// The previous section is the one before it in document order, taken as stated rather than as
    /// resolved: LibreOffice reads <c>GetLastSectionContext()</c>'s own property map, and every DOCX
    /// section states a <c>w:pgSz</c> or defaults to a portrait Letter. So this pass does not carry a
    /// resolved orientation forward the way the WW8 reader's <c>ResolveContinuousBreaks</c> carries a
    /// sheet — that reader implements a different importer's rule and is not this one.
    /// </para>
    /// </remarks>
    private static void PromoteContinuousAcrossOrientation(List<WritingSection> sections)
    {
        for (int i = 1; i < sections.Count; i++)
        {
            if (sections[i].Break == SectionBreak.Continuous
                && sections[i].Page.IsLandscape != sections[i - 1].Page.IsLandscape)
            {
                sections[i] = sections[i] with { Break = SectionBreak.NextPage };
            }
        }
    }

    /// <summary>
    /// Reads the header and footer parts the document's sections name.
    /// </summary>
    /// <remarks>
    /// Unlike ODF, a DOCX names its headers by relationship from each <c>w:sectPr</c>, so only
    /// the parts a section actually references are read — there is no risk of picking up an
    /// unused master. A part referenced by several sections is read once.
    /// </remarks>
    private static void ReadHeadersAndFooters(
        DocxFile file,
        XElement body,
        DocxContentReader reader,
        ContentDocument content)
    {
        HashSet<string> alreadyRead = new(StringComparer.Ordinal);
        int index = 0;

        foreach (XElement sectionProperties in DocxContentReader.SectionProperties(body))
        {
            foreach (XElement reference in sectionProperties.Elements())
            {
                bool isHeader = Word.Is(reference, "headerReference");
                if (!isHeader && !Word.Is(reference, "footerReference")) continue;

                string? relationshipId = Word.RelationshipId(reference);
                if (relationshipId is null) continue;

                if (file.Relationship(relationshipId) is { } relationship
                    && !alreadyRead.Add(relationship.Target))
                    continue;

                XElement? part = file.LoadHeaderOrFooter(relationshipId);
                if (part is null) continue;

                // Each flow numbers its own lists: a page number in a footer does not continue
                // the body's count.
                file.Numbering.ResetCounters();

                ContentSection section = new()
                {
                    Kind = isHeader ? SectionKind.Header : SectionKind.Footer,
                    Index = index,
                    // w:type distinguishes the default, first-page and even-page variants, and
                    // which one this is is the only thing that tells them apart in the output.
                    Name = Word.Attribute(reference, "type") ?? "default",
                };
                reader.ReadBlocks(part, section);

                if (section.Children.Count == 0) continue;

                content.Children.Add(section);
                foreach (ContentNode node in reader.TakeHoisted()) content.Children.Add(node);
                index++;
            }
        }

        // Restored so that a caller reading the numbering afterwards sees the body's state
        // rather than a header's.
        file.Numbering.ResetCounters();
    }
}

/// <summary>An OOXML word-processing document that has been read.</summary>
public sealed class OoxmlWordDocument : IWordProcessingDocument, IPaginatedDocument
{
    private readonly DocxFile _file;

    /// <summary>
    /// What laying the document out found, which reading it could not have.
    /// </summary>
    /// <remarks>
    /// A second list rather than an addition to the reader's, because the two are produced at different
    /// times: the reader's are answerable the moment the package is open, and a picture that will not
    /// draw is only discovered when something asks for the pages. So a caller that has run
    /// <see cref="Layout"/> sees more than one that has not, which is the honest answer — before layout
    /// nothing had looked.
    /// </remarks>
    private readonly List<Diagnostic> _laidOut = [];

    /// <summary>The file the document was read from, for a <c>FILENAME</c> field.</summary>
    private readonly string? _fileName;

    // `fileName` is the leaf name of the file the document was read from, or null when it came from a
    // stream. Only a FILENAME field wants it — see ConstantFields.
    internal OoxmlWordDocument(
        DocumentFormat format,
        DocxFile file,
        ContentDocument content,
        IReadOnlyList<Diagnostic> diagnostics,
        IReadOnlyList<WritingSection> sections,
        WritingMarks marks,
        string? fileName = null)
    {
        _fileName = fileName;
        Format = format;
        _file = file;
        Content = content;
        _read = diagnostics;
        Sections = sections.Count > 0 ? sections : [new WritingSection()];
        Marks = marks;
    }

    /// <inheritdoc/>
    public DocumentFormat Format { get; }

    /// <inheritdoc/>
    public DocumentFamily Family => DocumentFamily.WordProcessing;

    /// <inheritdoc/>
    public DocumentMetadata Metadata => Content.Metadata;

    /// <inheritdoc/>
    public ContentDocument Content { get; }

    /// <inheritdoc/>
    public IReadOnlyList<Diagnostic> Diagnostics
        => _laidOut.Count == 0 ? _read : [.. _read, .. _laidOut];

    private readonly IReadOnlyList<Diagnostic> _read;

    /// <inheritdoc/>
    public IReadOnlyList<WritingSection> Sections { get; }

    /// <inheritdoc/>
    public WritingMarks Marks { get; }

    /// <summary>
    /// The underlying package: its styles, numbering, settings and remaining parts.
    /// </summary>
    /// <remarks>
    /// Kept reachable because rendering will need the formatting the content tree deliberately
    /// discards, and because a caller wanting an embedded image's bytes has to reach the
    /// package. Valid until this document is disposed.
    /// </remarks>
    public DocxFile File => _file;

    /// <summary>
    /// Lays the document out into pages.
    /// </summary>
    /// <remarks>
    /// One section's geometry, the first, because the paginator takes one — a document that changes page
    /// setup part way through has its page break honoured and the geometry after it not. Carrying a
    /// section change needs each paragraph to know which section it is in, which is the gap recorded in
    /// this library's TODO.
    /// </remarks>
    public IPageSequence Layout(LayoutOptions? options = null)
        => XRay.Office.Core.Threading.OfficeOperation.Layout(options, () => LayoutCore(options));

    private WordProcessingPages LayoutCore(LayoutOptions? options)
    {
        XElement? body = _file.Body;
        if (body is null) return new WordProcessingPages([]);

        DocxLayoutSource source = new(
            _file.Styles, _file.Settings, footnotes: _file.Footnotes, endnotes: _file.Endnotes,
            theme: _file.Theme, shapeStyles: _file.ShapeStyles,
            pictures: new DocxPictures(_file, _laidOut),
            numbering: _file.Numbering, fontTable: _file.FontTable,
            constants: new ConstantFields(_fileName, Content.Metadata.Title))
        {
            // Only so the reader can tell a continuous break from a page-starting one when it hands a
            // dropped section mark's space-after on. See `DocxLayoutSource.HandOnBelowSpacing`.
            SectionBreaks = [.. Sections.Select(section => section.Break)],

            // The body's own relationships, which is what a hyperlink in the body names; the source
            // declines to use them for a header, a footer or a note. See `DocxLayoutSource.LinksResolvable`.
            LinkTarget = id => _file.Relationship(id) is { IsExternal: true } found ? found.Target : null,
        };
        List<PageBlock> blocks = source.Read(body);

        // The compatibility options, of which two reach pagination. Which ones those are was
        // settled by rendering documents with and without each flag rather than by reading the
        // schema; WordCompatibility records what every one of them was measured to do.
        WordCompatibility compatibility = WordCompatibility.Read(_file.Settings);

        PaginationOptions pagination = PaginationOptions.Word with
        {
            // `WriterFilter.cxx`:332 sets `DoNotCaptureDrawObjsOnPage` for every writerfilter import,
            // and a DOCX is one — so an anchored object here is not pulled back inside the page the
            // way a `.doc`'s or an ODF document's is. See
            // `PaginationOptions.CapturesAnchoredObjectsOnPage`.
            CapturesAnchoredObjectsOnPage = false,

            // ...but that flag is only one term of the condition. `bConsidered`
            // (`anchoredobjectposition.cxx`:125-144) exempts a wrap-through fly and a *shape with no
            // text box*, and nothing else — so a DOCX picture or text box stating `wp:wrapSquare` is
            // still pulled back inside its area. See `PaginationOptions.CapturesWrappedObjects`.
            CapturesWrappedObjects = true,

            // And under `compatibilityMode` 15 that area is the page's *body* rather than the sheet,
            // wherever the anchor has a body frame to be narrowed to — which a header or footer anchor
            // has not. `PaginationOptions.NarrowsCaptureToBody` has the five conditions and the two
            // documents the missing one costs.
            NarrowsCaptureToBody = compatibility.CompatibilityMode >= 15,

            // `WriterFilter.cxx`:333 sets `DisableOffPagePositioning` one line below the flag above,
            // and it is the other half of `SwAnchoredObject::IsDraggingOffPageAllowed` — so a DOCX's
            // wrap-through object is the one kind `SwFlyFreeFrame::CheckClip` leaves alone, at
            // whatever size it states. See `PaginationOptions.DisablesOffPagePositioning`.
            //
            // **It is DOCX's alone and not the writerfilter's.** That file is the OOXML filter;
            // `Rich_Text_Format.xcu` names `com.sun.star.comp.Writer.RtfFilter`, and that class'
            // `setTargetDocument` (`RtfFilter.cxx`:191-195) sets no property at all. Grepping `sw/`
            // for the setting finds exactly one setter. So the RTF reader leaves this false and its
            // oversize flies are squeezed whatever they wrap, which is what the reference does.
            // (`probes/chart-fit-r97` §3; the round that salvaged this had the prose the other way
            // round and the code this way.)
            DisablesOffPagePositioning = true,

            // LibreOffice's PARA_SPACE_MAX means the two spacings *add*; when it is off the larger
            // wins, which is Word's behaviour. Its OOXML exporter writes
            // w:doNotUseHTMLParagraphAutoSpacing exactly when the flag is on (docxexport.cxx), so
            // the element's absence is what makes a DOCX collapse — and a document carrying it
            // adds, like an ODF one. A package with no settings part at all is the exception,
            // because the write that decides this never runs; WordCompatibility.AddsParagraphSpacing
            // says why, and measures it.
            CollapsesSpacing = !compatibility.AddsParagraphSpacing,

            // `SwFrame::IsCollapseUpper`, gated on `TAB_OVER_SPACING && !TAB_OVER_MARGIN` — which is
            // `compatibilityMode` 15 or more, since `SettingsTable.cxx`:685 sets `TabOverMargin` at 14
            // and below and an absent mode means 12. From Word 2013 a paragraph at the top of any page
            // but the first drops its space-before even when it is the paragraph that broke the page.
            CollapsesUpperAtPageTop = compatibility.CompatibilityMode >= 15,

            // The same flag the other way up — `TAB_OVER_MARGIN`, set at `compatibilityMode` 14 and
            // below. See `PaginationOptions.FliesMayOverlapTheBottomMargin`, which measures it.
            FliesMayOverlapTheBottomMargin = compatibility.CompatibilityMode < 15,

            JustifiesLinesEndedByBreak = !compatibility.DoNotExpandShiftReturn,

            // `WriterFilter::setTargetDocument` sets `ContinuousEndnotes` on every package it opens,
            // under a comment reading "options that are valid for the DOCX format" — so this is not
            // read from the document, exactly as `AddsCellLineSpacing` above is not. The RTF reader
            // shares `PaginationOptions.Word` and must *not* set it; see the flag's own remarks.
            UsesWordNoteSeparator = true,

            // The same two filters, the same place, one line further down: `setTargetDocument` also sets
            // `ContinuousEndnotes`, which puts the document's endnotes after its last body content rather
            // than on pages of their own. 26.2.4.2 sets it and 24.2.7.2 did not — see the flag's remarks
            // for the measurement, which is a page count on this repository's own `endnotes.docx`.
            EndnotesFollowTheBody = true,
            // Zero means no face could be read, in which case Writer's fixed reservation is a better
            // answer than reserving nothing: a note area with no room above it would overprint.
            NoteSeparatorHeight = source.DefaultParagraphLineHeight > Core.Units.Length.Zero
                ? source.DefaultParagraphLineHeight
                : PaginationOptions.Word.NoteSeparatorHeight,

            MaxPages = options?.MaxPages is > 0
                ? options.MaxPages
                : PaginationOptions.Word.MaxPages,

            // Null for all but one document in the words corpus, and the read is one pass over the
            // `w:sectPr` elements — see `DocxLayoutSource.LineNumbers` for why the last one wins.
            LineNumbers = source.LineNumbers(body),
        };

        Paginator paginator = new(pagination);

        return new WordProcessingPages(
            paginator.Paginate(blocks, Paginated(source, body)),
            paginator.Blocks ?? blocks);
    }

    /// <summary>
    /// The document's sections, each paired with the furniture its own <c>w:sectPr</c> names.
    /// </summary>
    /// <remarks>
    /// Per section rather than the first section's throughout, because a document that changes its page
    /// setup halfway usually changes its headers with it — a landscape appendix with its own running head is
    /// the ordinary case. The enumeration order matches <see cref="Sections"/>, which is what
    /// <see cref="PageBlock.SectionIndex"/> indexes.
    /// </remarks>
    private List<PaginatedSection> Paginated(DocxLayoutSource source, XElement body)
    {
        List<XElement> properties = [.. DocxContentReader.SectionProperties(body)];
        List<PaginatedSection> sections = new(Sections.Count);

        // Carried across the sections: a slot a section names nothing for keeps whatever the section
        // before it had. That is §17.10.1's rule — "if no headerReference is specified, the header is
        // inherited from the previous section" — and it is what "link to previous" writes, so a document
        // with one running head over several sections states it once. Losing it leaves every section
        // after the first with no running head, and a page with no header holds more lines than it should.
        FurnitureCarry headers = new();
        FurnitureCarry footers = new();

        for (int i = 0; i < Sections.Count; i++)
        {
            bool statesOwn = false;
            PageFurnitureSet? furniture = i < properties.Count
                ? Furniture(source, properties[i], headers, footers, out statesOwn)
                : null;

            sections.Add(new PaginatedSection(Sections[i], furniture, statesOwn));
        }

        return sections;
    }

    /// <summary>
    /// One section's headers and footers, laid out from the parts its <c>w:sectPr</c> names.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Read through the same walk the body uses, because a header's paragraphs are paragraphs: they
    /// resolve their styles the same way, measure the same way, and a second walk would be a second place
    /// for the run and tab handling to be got right. A <c>w:hdr</c> root holds block-level children just
    /// as <c>w:body</c> does, so the walk needs no special case.
    /// </para>
    /// <para>
    /// A DOCX names its furniture by relationship and labels each with a <c>w:type</c>, so — unlike ODF
    /// — an unreferenced part is never picked up. The <c>w:type</c> is the only thing distinguishing the
    /// three variants; a reference without one is the default.
    /// </para>
    /// </remarks>
    /// <param name="source">The walk the header's paragraphs are read through.</param>
    /// <param name="sectionProperties">The section's <c>w:sectPr</c>.</param>
    /// <param name="headers">
    /// The headers in force, by slot, carried across the sections and updated in place: a slot this
    /// section names replaces what was there, and one it does not name is inherited.
    /// </param>
    /// <param name="footers">The footers in force, by the same rule.</param>
    /// <param name="statesOwnFurniture">
    /// Set true when the <c>w:sectPr</c> names a reference of its own for any slot, which is the
    /// negation of writerfilter's six <c>m_b*LinkToPrevious</c> flags and the whole of what decides
    /// whether a continuous section gets a page descriptor at all — see
    /// <see cref="Layout.ContinuousPageDescriptors"/>. A reference naming an empty part still counts:
    /// "link to previous" is the absence of a reference, and the importer clears the flag in
    /// <c>PopPageHeaderFooter</c> for every reference it reads.
    /// </param>
    private PageFurnitureSet? Furniture(
        DocxLayoutSource source,
        XElement sectionProperties,
        FurnitureCarry headers,
        FurnitureCarry footers,
        out bool statesOwnFurniture)
    {
        statesOwnFurniture = false;

        // Before this section names anything of its own, drop the slots the section above cannot pass
        // down — see <see cref="FurnitureCarry"/> for which those are and how the rule was measured.
        headers.DropUninheritable();
        footers.DropUninheritable();

        foreach (XElement reference in sectionProperties.Elements())
        {
            bool isHeader = Word.Is(reference, "headerReference");
            if (!isHeader && !Word.Is(reference, "footerReference")) continue;

            if (SlotOf(Word.Attribute(reference, "type")) is not { } slot) continue;

            statesOwnFurniture = true;
            string? relationshipId = Word.RelationshipId(reference);
            if (_file.LoadHeaderOrFooter(relationshipId) is not { } part) continue;

            // The furniture part's own relationships for the duration of its walk, and the main
            // document's afterwards. A header's pictures index its own `.rels`, whose ids start again
            // at one — see the remarks on `DocxPictures`, where getting this wrong is silent.
            string? scope = source.Pictures?.Scope;
            if (source.Pictures is { } pictures && _file.Relationship(relationshipId) is { } named)
            {
                pictures.Scope = named.Target;
            }

            List<PageBlock> blocks;
            source.InHeaderFooter = true;
            try
            {
                blocks = source.ReadFlow(part);
            }
            finally
            {
                source.InHeaderFooter = false;
                if (source.Pictures is { } restore && scope is not null) restore.Scope = scope;
            }

            // A reference settles the slot even when the part it names is empty: "link to previous" is
            // the *absence* of a reference, so a section pointing at an empty header is saying it has
            // none rather than asking for the one above.
            FurnitureCarry into = isHeader ? headers : footers;
            if (blocks.Count == 0) into.Clear(slot);
            else into.Set(slot, blocks);
        }

        PageFurnitureSet set = new(headers.InForce, footers.InForce);
        return set.IsEmpty ? null : set;
    }

    /// <summary>
    /// The headers — or footers — in force as the sections are walked, and which of them the next
    /// section may inherit.
    /// </summary>
    /// <remarks>
    /// <para>
    /// §17.10.1 says a section naming no reference for a slot keeps the one above it, and Word does
    /// exactly that. <b>LibreOffice does not, when the part it would pass down holds no paragraph of its
    /// own</b> — a <c>w:hdr</c> whose only children are <c>w:tbl</c>. It copies the previous section's
    /// header *content* between page styles rather than linking the two
    /// (<c>sw/source/writerfilter/dmapper/PropertyMap.cxx</c>, <c>copyHeaderFooterTextProperty</c>), and
    /// a body of text that begins and ends with a table copies as nothing at all, so the inheriting
    /// section is left with no running head and reserves no space for one.
    /// </para>
    /// <para>
    /// Measured against 26.2.4.2 on a two-section probe, varying only the header part's own children:
    /// one table alone and two tables alone are not passed down; the same table with an empty
    /// <c>w:p</c> before it, the same table with one after it, and a bare paragraph all are. Round 43
    /// had established the same rule against 24.2.7.2 from both ends —
    /// <c>probes/words-r43/header-inherit-bisect.py</c> and <c>header-inherit-content-shape.py</c> —
    /// and chose not to reproduce it. That decision and its reversal are written out on
    /// <c>SectionInheritedHeaderTests</c>, which asserted the opposite until 2026-08-15 and is the place
    /// to read before turning this off again. The document it was found on is
    /// <c>words/extra-001/docx/UG.CAO.00133 Foreign Part 145 approvals - Language.docx</c>, whose
    /// header part is one nested table ending <c>&lt;/w:tbl&gt;&lt;/w:hdr&gt;</c>: the reference draws
    /// its running head on the five pages whose own section names it and on none of the other thirteen.
    /// </para>
    /// <para>
    /// It is all-or-nothing on the presence of a paragraph rather than a filter on the blocks: where a
    /// paragraph is there, the tables beside it travel with it.
    /// </para>
    /// </remarks>
    private sealed class FurnitureCarry
    {
        private readonly HashSet<PageFurnitureSlot> _uninheritable = [];

        /// <summary>What each slot holds for the section being read.</summary>
        public Dictionary<PageFurnitureSlot, IReadOnlyList<PageBlock>> InForce { get; } = [];

        /// <summary>Records the part a section names for one slot.</summary>
        public void Set(PageFurnitureSlot slot, IReadOnlyList<PageBlock> blocks)
        {
            InForce[slot] = blocks;
            bool inheritable = false;
            foreach (PageBlock block in blocks)
            {
                if (block is not PageParagraph) continue;
                inheritable = true;
                break;
            }

            if (inheritable) _uninheritable.Remove(slot);
            else _uninheritable.Add(slot);
        }

        /// <summary>Records that a section named an empty part for one slot, so it has none.</summary>
        public void Clear(PageFurnitureSlot slot)
        {
            InForce.Remove(slot);
            _uninheritable.Remove(slot);
        }

        /// <summary>Drops the slots the section above cannot pass down. Called once per section.</summary>
        public void DropUninheritable()
        {
            foreach (PageFurnitureSlot slot in _uninheritable) InForce.Remove(slot);
            _uninheritable.Clear();
        }
    }

    /// <summary>
    /// The slot a <c>w:type</c> names, or null for a value this reader does not know.
    /// </summary>
    /// <remarks>
    /// Null rather than falling back to the default slot, because a producer writing an unknown type
    /// means something by it, and quietly filing it as the default would put that content on every page
    /// of the document.
    /// </remarks>
    private static PageFurnitureSlot? SlotOf(string? type) => type switch
    {
        null or "" or "default" => PageFurnitureSlot.Default,
        "first" => PageFurnitureSlot.First,
        "even" => PageFurnitureSlot.Even,
        _ => null,
    };

    /// <inheritdoc/>
    public void Dispose() => _file.Dispose();
}
