using XRay.Office.Core.Units;
using XRay.Office.OpenDocument;
using XRay.Office.OpenDocument.Styles;
using XRay.Office.WordProcessing.Model;

namespace XRay.Office.WordProcessing.OpenDocument;

/// <summary>
/// What a <c>text:section</c> does to the geometry of the text inside it: its columns and its indents.
/// </summary>
/// <remarks>
/// <para>
/// A <c>text:section</c> is Writer's own <c>SwSectionFrame</c>, and it is the object every Word-family
/// importer reaches for when a section break has to change the column count without starting a page.
/// <c>SectionPropertyMap::CloseSectionGroup</c> says so outright — <em>"prefer setting column properties
/// into a section, not a page style if at all possible"</em>, then
/// <c>xSection = rDM_Impl.appendTextSectionAfter(m_xStartingRange)</c>
/// (<c>sw/source/writerfilter/dmapper/PropertyMap.cxx</c>:1905-1913) — and the WW8 importer builds the
/// same object in <c>wwSectionManager::InsertSection</c>, where the section's own indents are the
/// <em>difference</em> between the Word section's page margins and the page style's:
/// <c>nSectionLeft = rSection.GetPageLeft() - nPageLeft</c>, put on the section format as an
/// <c>SvxLRSpaceItem</c> before <c>SetCols</c> (<c>sw/source/filter/ww8/ww8par6.cxx</c>:735-745).
/// LibreOffice's ODF export writes that back out as a section-family style carrying
/// <c>style:columns</c> and <c>fo:margin-left</c>/<c>fo:margin-right</c>, so reading it here is the exact
/// inverse of what produced the file.
/// </para>
/// <para>
/// The indents are stated relative to the page's own margins, which is why they are returned as a
/// difference and added to the master's rather than replacing them.
/// </para>
/// <para>
/// A section that states one column and no indents changes nothing about the layout, and
/// <see cref="Read"/> answers null for it — most of the corpus's sections are that, written by the
/// exporter to carry a protection flag or a name.
/// </para>
/// </remarks>
internal readonly record struct OdfSectionGeometry
{
    /// <summary>How many columns the section's text is laid out in.</summary>
    public int Columns { get; init; }

    /// <summary>The gap between those columns.</summary>
    public Length ColumnGap { get; init; }

    /// <summary>
    /// The <c>style:section-properties</c> the columns were read from, kept so that the per-column
    /// widths can be apportioned once the section's own measure is known.
    /// </summary>
    /// <remarks>
    /// A <c>style:rel-width</c> is a proportion of the width the columns have to fill, and that width is
    /// the page's less this section's own indents — which only the caller that owns the master page can
    /// work out. See <see cref="RulerFor"/> and <see cref="OdfPageGeometry.ColumnRulerOf"/>.
    /// </remarks>
    public OdfPropertySet? Properties { get; init; }

    /// <summary>The columns as the file states them one by one, apportioned to a measure.</summary>
    /// <param name="measure">The width the section's columns have to fill.</param>
    public ColumnRuler? RulerFor(Length measure)
        => OdfPageGeometry.ColumnRulerOf(Properties, measure);

    /// <summary>How far the section's text is indented from the page's left margin.</summary>
    public Length IndentLeft { get; init; }

    /// <summary>How far it is indented from the page's right margin.</summary>
    public Length IndentRight { get; init; }

    /// <summary>
    /// True when the section's columns are balanced rather than filled in turn.
    /// </summary>
    /// <remarks>
    /// ODF states the negative — <c>text:dont-balance-text-columns</c>, which is
    /// <c>SwFormatNoBalancedColumns</c> — so an absent attribute means balanced, which is Writer's own
    /// answer for a section it created. Both Word importers set the flag for a section a page break
    /// follows, so a file converted from a Word document usually states it.
    /// </remarks>
    public bool BalancesColumns { get; init; }

    /// <summary>
    /// Reads a <c>text:section</c>'s geometry, or null when it leaves the layout alone.
    /// </summary>
    /// <param name="styles">The document's resolved styles.</param>
    /// <param name="styleName">The section's <c>text:style-name</c>.</param>
    internal static OdfSectionGeometry? Read(OdfStyles styles, string? styleName)
    {
        ArgumentNullException.ThrowIfNull(styles);

        if (styleName is null) return null;

        OdfPropertySet? properties = Resolve(styles, styleName);
        if (properties is null) return null;

        int columns = OdfPageGeometry.ColumnCount(properties);
        Length left = Indent(properties, "margin-left");
        Length right = Indent(properties, "margin-right");

        if (columns <= 1 && left == Length.Zero && right == Length.Zero) return null;

        return new OdfSectionGeometry
        {
            Columns = columns,
            ColumnGap = OdfPageGeometry.ColumnGap(properties),
            Properties = properties,
            IndentLeft = left,
            IndentRight = right,
            BalancesColumns =
                properties.Get(OdfNamespaces.Text, "dont-balance-text-columns") != "true",
        };
    }

    /// <summary>
    /// The geometry of a section nested inside <paramref name="parent"/>, which is never null.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A section inside a section is not laid out inside its parent's columns. Writer inserts the child's
    /// frame <em>behind</em> the parent, into the parent's own upper —
    /// <c>pFrame-&gt;InsertBehind(pTmp-&gt;GetUpper(), pTmp)</c>,
    /// <c>sw/source/core/layout/frmtool.cxx</c>:1795-1803 — and splits the parent at the child's end node
    /// so that what follows is a second frame of the parent's format (<c>SwSectionFrame::SplitSect</c>,
    /// <c>:1954-1960</c>). The parent's <c>SwColumnFrame</c>s are inside the parent's own frame, so a
    /// sibling never sees them.
    /// </para>
    /// <para>
    /// What the child <em>does</em> take from the parent is the indents, and the asymmetry has a seat of
    /// its own. A nested section's format is derived from the enclosing section's
    /// (<c>pFormat-&gt;SetDerivedFrom(pSectNd ? pSectNd-&gt;GetSection().GetFormat() : …)</c>,
    /// <c>sw/source/core/docnode/ndsect.cxx</c>:1345), so <c>GetLRSpace()</c> finds the parent's item
    /// where the child states none — while <c>GetCol()</c> never does, because
    /// <c>SwSectionFormat</c>'s constructor puts the pool's default one-column item on every section
    /// format outright (<c>SetFormatAttr(*GetDfltAttr(RES_COL))</c>,
    /// <c>sw/source/core/docnode/section.cxx</c>:608-614). <c>SwSectionFrame::Init</c> then takes its
    /// width from <c>GetUpper()-&gt;getFramePrintArea()</c> and insets it by that <c>GetLRSpace()</c>
    /// (<c>sectfrm.cxx</c>:129-166).
    /// </para>
    /// <para>
    /// The indents are one <c>SvxLRSpaceItem</c> and not two properties, so a child stating either
    /// <c>fo:margin-left</c> or <c>fo:margin-right</c> replaces the pair and the side it left out is
    /// nought. Measured on 26.2.4.2, one variant per arm: a child stating only a 1.5 in left indent
    /// inside a parent indented 0.5 in and 0.75 in is drawn from 180.1 pt to the page's own right edge,
    /// not to the parent's. <c>probes/odt-sectable-r92/nested.py</c>.
    /// </para>
    /// </remarks>
    /// <param name="styles">The document's resolved styles.</param>
    /// <param name="styleName">The nested section's <c>text:style-name</c>, which may be absent.</param>
    /// <param name="parent">The geometry of the section it is nested in.</param>
    internal static OdfSectionGeometry Nested(
        OdfStyles styles, string? styleName, OdfSectionGeometry parent)
    {
        ArgumentNullException.ThrowIfNull(styles);

        OdfPropertySet? properties = styleName is null ? null : Resolve(styles, styleName);

        bool statesIndent =
            properties is not null
            && (properties.Get(OdfNamespaces.FoCompatible, "margin-left") is not null
                || properties.Get(OdfNamespaces.FoCompatible, "margin-right") is not null);

        return new OdfSectionGeometry
        {
            Columns = properties is null ? 1 : OdfPageGeometry.ColumnCount(properties),
            ColumnGap = properties is null ? Length.Zero : OdfPageGeometry.ColumnGap(properties),
            Properties = properties,
            IndentLeft = statesIndent ? Indent(properties!, "margin-left") : parent.IndentLeft,
            IndentRight = statesIndent ? Indent(properties!, "margin-right") : parent.IndentRight,
            BalancesColumns =
                properties?.Get(OdfNamespaces.Text, "dont-balance-text-columns") != "true",
        };
    }

    /// <summary>
    /// The section properties a style name resolves to, following the parent chain.
    /// </summary>
    /// <remarks>
    /// A section style may be derived like any other, and the properties are taken from the nearest
    /// ancestor that states a <c>style:section-properties</c> at all — the same shape of resolution the
    /// paragraph and page readers already do, and cycle-guarded for the same reason: a style pool read
    /// from a file can point at itself.
    /// </remarks>
    private static OdfPropertySet? Resolve(OdfStyles styles, string styleName)
    {
        HashSet<string> seen = new(StringComparer.Ordinal);
        string? name = styleName;

        for (int depth = 0; depth < 64 && name is not null && seen.Add(name); depth++)
        {
            OdfStyle? style = styles.Find(name, OdfStyleFamily.Section);
            if (style is null) return null;

            if (style.Properties(OdfPropertyKind.Section) is { } properties) return properties;

            name = style.ParentStyleName;
        }

        return null;
    }

    /// <summary>One of the section's two indents, floored at nought.</summary>
    /// <remarks>
    /// A negative indent is legal in ODF and Writer honours it, but nothing this reader hands the
    /// paginator can take a text area wider than the body — <see cref="OdfSectionGeometry"/> is added to
    /// the page's margins and a wider one would run under the header's own edge. Floored rather than
    /// dropped, so the other side of a one-sided negative pair still applies.
    /// </remarks>
    private static Length Indent(OdfPropertySet properties, string localName)
    {
        Length? stated = OdfWriterUnits.ToCore(
            OdfValue.ParseLength(properties.Get(OdfNamespaces.FoCompatible, localName)));

        return stated is { } value && value > Length.Zero ? value : Length.Zero;
    }
}

/// <summary>
/// One section the walk derived from a <c>text:section</c>: which master it sits on, and what the
/// section does to that master's geometry.
/// </summary>
/// <remarks>
/// A null <paramref name="Geometry"/> is the section that <em>closes</em> a <c>text:section</c> — the
/// master's own geometry again, reached by a continuous break rather than by returning to the master's
/// own section, whose break would start a page.
/// </remarks>
/// <param name="Master">The master page the section is laid out on.</param>
/// <param name="Geometry">What the <c>text:section</c> changes, or null to restore the master's.</param>
internal readonly record struct OdtColumnSection(int Master, OdfSectionGeometry? Geometry);
