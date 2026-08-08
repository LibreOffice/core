using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.WordProcessing.Layout;
using Paperless.WordProcessing.Model;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// A <c>PAGE</c> field in a running head prints the page's own number, not the producer's cache.
/// </summary>
/// <remarks>
/// <para>
/// The defect these were written against: every reader parsed the field, recorded its kind, and laid out
/// the string the writing application had cached — so <c>batch-010/195584360.docx</c> printed "Page 10"
/// on all twenty of its pages, and <c>batch-019/150_5335_5a.doc</c> printed 13 on page 9 where the
/// reference prints 3.
/// </para>
/// <para>
/// Each of these was checked by putting the defect back and watching it fail; the note on
/// <see cref="ARunningHeadWithNoFieldIsStillLaidOutOnce"/> says which ones a given reintroduction moves,
/// because that one is a control rather than a detector.
/// </para>
/// </remarks>
public sealed class PageNumberFieldTests
{
    /// <summary>Each page's footer carries its own number, and the cached one appears nowhere.</summary>
    [Fact]
    public void EveryPageGetsItsOwnNumber()
    {
        List<LaidOutPage> pages = Paginate(Footer("Page 10", field: (5, 2)));

        pages.Count.ShouldBe(3);
        FooterText(pages[0]).ShouldBe("Page 1");
        FooterText(pages[1]).ShouldBe("Page 2");
        FooterText(pages[2]).ShouldBe("Page 3");
    }

    /// <summary>
    /// The number is measured, not painted over: a footer whose cached result was two digits wide is
    /// re-laid-out at the width of the one-digit number that replaces it.
    /// </summary>
    /// <remarks>
    /// The whole reason the substitution happens before <c>FlowLayouter</c> rather than after it. A
    /// centred or right-aligned footer painted over would sit half a digit's advance out on every page
    /// whose number has a different width from the cached one.
    /// </remarks>
    [Fact]
    public void TheNumberTakesPartInMeasurement()
    {
        List<LaidOutPage> pages = Paginate(Footer("Page 10", field: (5, 2)));

        Length one = LineWidth(pages[0]);
        Length ten = LineWidth(pages[9 % pages.Count]);

        one.ShouldBeLessThan(ten + Length.FromPoints(0.01));

        // And the substituted line really is narrower than the cached one it replaced, which is what
        // says the measurement used the new string.
        Length cached = LineWidth(Paginate(Footer("Page 10"))[0]);
        one.ShouldBeLessThan(cached);
    }

    /// <summary>The section's sequence decides how the number is written.</summary>
    /// <remarks>
    /// <c>w:pgNumType w:fmt="lowerRoman"</c> and <c>sprmSNfcPgn</c> of 2. 21 of the corpus's 134 DOCX
    /// declare a <c>lowerRoman</c> section, so this is not a synthetic case.
    /// </remarks>
    [Fact]
    public void TheSectionsSequenceIsHonoured()
    {
        List<LaidOutPage> pages = Paginate(
            Footer("7", field: (0, 1)), NoteNumberFormat.LowerRoman);

        FooterText(pages[0]).ShouldBe("i");
        FooterText(pages[1]).ShouldBe("ii");
        FooterText(pages[2]).ShouldBe("iii");
    }

    /// <summary>A field's own picture switch overrides the section's sequence.</summary>
    [Fact]
    public void AFieldsOwnPictureWins()
    {
        List<LaidOutPage> pages = Paginate(
            Footer("7", field: (0, 1), format: NoteNumberFormat.UpperRoman),
            NoteNumberFormat.Arabic);

        FooterText(pages[1]).ShouldBe("II");
    }

    /// <summary>
    /// A section that restarts its numbering restarts what the field prints.
    /// </summary>
    /// <remarks>
    /// The half of the brief this was written to test: our page 9 of <c>150_5335_5a.doc</c> said 13
    /// where the reference said 3, and the diagnosis on record was that the section's restart was wrong
    /// as well as the cache. It was not — the field simply never asked what page it was on.
    /// </remarks>
    [Fact]
    public void ARestartedSectionRestartsTheField()
    {
        List<LaidOutPage> pages = Paginate(Footer("Page 10", field: (5, 2)), restartAt: 5);

        FooterText(pages[0]).ShouldBe("Page 5");
        FooterText(pages[2]).ShouldBe("Page 7");
    }

    /// <summary>Run formatting on either side of the field survives the substitution.</summary>
    /// <remarks>
    /// The splice has to rebuild the run partition, because the replacement is a different length from
    /// the text it replaced and every offset after it moves. A footer whose number is bold and whose
    /// surrounding words are not is the ordinary shape of "Page 3 of 9".
    /// </remarks>
    [Fact]
    public void RunsAroundTheFieldKeepTheirText()
    {
        PageParagraph paragraph = Footer("Page 10 of 12", field: (5, 2))[0] as PageParagraph
            ?? throw new InvalidOperationException("the footer is a paragraph");

        PageParagraph resolved =
            (PageParagraph)PageFields.Resolve([paragraph], 3, NoteNumberFormat.Arabic)[0];

        resolved.Text.ShouldBe("Page 3 of 12");

        // Three runs: "Page ", the number, " of 12" — with the number taking the formatting of the run
        // the cached result started in, and the tail moved by the change in length.
        resolved.Runs.Count.ShouldBe(3);
        resolved.Runs[0].Start.ShouldBe(0);
        resolved.Runs[1].Start.ShouldBe(5);
        resolved.Runs[1].Length.ShouldBe(1);
        resolved.Runs[1].Colour.ShouldBe(Colour.FromRgb(0xC00000));
        resolved.Runs[2].Start.ShouldBe(6);
        resolved.Text[resolved.Runs[2].Start..].ShouldBe(" of 12");
    }

    /// <summary>
    /// Two page-number fields in one paragraph both resolve, and the second one's offsets survive the
    /// first one's substitution.
    /// </summary>
    /// <remarks>
    /// A header stating the number at both ends of a tab is the ordinary case. The spans are applied
    /// from the end backwards for exactly this: applying the first one moves everything after it, so a
    /// forward walk splices the second at an offset that is already stale — and with a two-digit cached
    /// result and a one-digit page number the error is a character, which lands inside a word.
    /// </remarks>
    [Fact]
    public void TwoFieldsInOneParagraphBothResolve()
    {
        PageParagraph paragraph = new()
        {
            Text = "10 of many, page 10",
            Face = Face,
            EmSize = Length.FromPoints(11),
            Fields =
            [
                new PageFieldSpan(0, 2, PageFieldKind.PageNumber),
                new PageFieldSpan(17, 2, PageFieldKind.PageNumber),
            ],
        };

        PageParagraph resolved =
            (PageParagraph)PageFields.Resolve([paragraph], 4, NoteNumberFormat.Arabic)[0];

        resolved.Text.ShouldBe("4 of many, page 4");
    }

    /// <summary>A running head with no field is laid out once and shared by every page.</summary>
    /// <remarks>
    /// <strong>A control, not a detector.</strong> Reintroducing any of the twelve defects tried against
    /// these tests leaves it passing, because the thing it asserts — that the cache is still keyed on the
    /// slot alone when nothing varies — is what every wrong version also does. It is kept because the
    /// substitution's cost is paid per page and a regression that keyed *every* header on its page number
    /// would be invisible to every other test here, and it is labelled so that nobody reads it as
    /// evidence the substitution works.
    /// </remarks>
    [Fact]
    public void ARunningHeadWithNoFieldIsStillLaidOutOnce()
    {
        PageFurnitureSet furniture = new(
            footers: new Dictionary<PageFurnitureSlot, IReadOnlyList<PageBlock>>
            {
                [PageFurnitureSlot.Default] = Footer("Page 10"),
            });

        WritingSection section = new() { Page = Geometry };

        PlacedFlow? first = furniture.Footer(section, Geometry, 1, isFirstPageOfSection: true);
        PlacedFlow? second = furniture.Footer(section, Geometry, 2, isFirstPageOfSection: false);

        first.ShouldNotBeNull();
        second.ShouldBeSameAs(first);
    }

    /// <summary>A span the reader recorded outside the text is ignored rather than throwing.</summary>
    /// <remarks>
    /// Readers record spans against a text that later passes rewrite — the case map, the list label — and
    /// a malformed file can put a field's separator after its end. Neither should cost the page.
    /// </remarks>
    [Fact]
    public void AnOutOfRangeSpanIsIgnored()
    {
        PageParagraph paragraph = new()
        {
            Text = "Page 10",
            Face = Face,
            EmSize = Length.FromPoints(11),
            Fields = [new PageFieldSpan(5, 99, PageFieldKind.PageNumber)],
        };

        PageParagraph resolved =
            (PageParagraph)PageFields.Resolve([paragraph], 3, NoteNumberFormat.Arabic)[0];

        resolved.Text.ShouldBe("Page 10");
    }

    /// <summary>A <c>NUMPAGES</c> span is recorded and deliberately left at its cached value.</summary>
    [Fact]
    public void APageCountFieldIsLeftAlone()
    {
        PageParagraph paragraph = new()
        {
            Text = "of 12",
            Face = Face,
            EmSize = Length.FromPoints(11),
            Fields = [new PageFieldSpan(3, 2, PageFieldKind.PageCount)],
        };

        PageFields.CarriesPageNumber([paragraph]).ShouldBeFalse();

        PageParagraph resolved =
            (PageParagraph)PageFields.Resolve([paragraph], 3, NoteNumberFormat.Arabic)[0];

        resolved.Text.ShouldBe("of 12");
    }

    /// <summary>A <c>NUMPAGES</c> field prints the number of pages the document turned out to have.</summary>
    /// <remarks>
    /// The second half of the same defect, and the one the previous round recorded as unfinished: the
    /// total is not known while the running head is being laid out, so the document is filled once to
    /// learn it and filled again to print it.
    /// </remarks>
    [Fact]
    public void APageCountFieldPrintsTheDocumentsOwnTotal()
    {
        List<LaidOutPage> pages = Paginate(Footer("of 12", field: (3, 2), kind: PageFieldKind.PageCount));

        pages.Count.ShouldBe(3);
        foreach (LaidOutPage page in pages) FooterText(page).ShouldBe("of 3");
    }

    /// <summary>A footer holding both fields resolves both, each to its own value.</summary>
    /// <remarks>
    /// The ordinary shape — "Page 1 of 3" — and the one that catches a resolver that substitutes the
    /// same value into every span it finds.
    /// </remarks>
    [Fact]
    public void ANumberAndACountInOneFooterBothResolve()
    {
        List<LaidOutPage> pages = Paginate(
            Footer(
                "Page 7 of 12",
                field: (5, 1),
                second: (10, 2),
                secondKind: PageFieldKind.PageCount));

        FooterText(pages[0]).ShouldBe("Page 1 of 3");
        FooterText(pages[2]).ShouldBe("Page 3 of 3");
    }

    /// <summary>A head holding only a page count is still laid out once, not once per page.</summary>
    /// <remarks>
    /// A page count is the same on every page, so it must not reach the layout cache's key. Keying on it
    /// would re-shape the running head once per page of every document that carries one, for an answer
    /// that cannot change — and this asserts the *sharing* rather than the timing, because a shared flow
    /// is the observable consequence of a shared cache entry.
    /// </remarks>
    [Fact]
    public void AHeadHoldingOnlyACountIsSharedAcrossPages()
    {
        List<LaidOutPage> pages = Paginate(Footer("of 12", field: (3, 2), kind: PageFieldKind.PageCount));

        pages[0].Footer.ShouldBeSameAs(pages[1].Footer);
    }

    /// <summary>A page number inside a table cell in the running head is resolved too.</summary>
    /// <remarks>
    /// A header laid out as a table is the ordinary way a Word document puts a logo beside a page number,
    /// so the resolver has to recurse rather than look at top-level paragraphs alone.
    /// </remarks>
    [Fact]
    public void ANumberInsideAHeaderTableIsResolved()
    {
        PageTable table = new()
        {
            ColumnWidths = [Length.FromTwips(4000)],
            Rows =
            [
                new PageTableRow
                {
                    Cells =
                    [
                        new PageTableCell { Blocks = Footer("Page 10", field: (5, 2)) },
                    ],
                },
            ],
        };

        PageFields.CarriesPageNumber([table]).ShouldBeTrue();

        PageTable resolved = (PageTable)PageFields.Resolve([table], 4, NoteNumberFormat.Arabic)[0];
        PageParagraph inside = (PageParagraph)resolved.Rows[0].Cells[0].Blocks[0];

        inside.Text.ShouldBe("Page 4");
    }

    private static string FooterText(LaidOutPage page)
    {
        page.Footer.ShouldNotBeNull();
        return string.Concat(page.Footer!.Blocks.OfType<PageParagraph>().Select(p => p.Text));
    }

    private static Length LineWidth(LaidOutPage page)
    {
        page.Footer.ShouldNotBeNull();
        return page.Footer!.Lines[0].Box.Width;
    }

    /// <summary>
    /// A footer paragraph, optionally with a page-number field over part of its text.
    /// </summary>
    /// <param name="text">The text, with the producer's cached result already in it.</param>
    /// <param name="field">Where that cached result sits, or null for a footer with no field.</param>
    /// <param name="format">The field's own picture switch, or null to take the section's.</param>
    /// <param name="kind">What the first field computes.</param>
    /// <param name="second">A second field's span, for a footer holding both a number and a count.</param>
    /// <param name="secondKind">What that second field computes.</param>
    private static IReadOnlyList<PageBlock> Footer(
        string text,
        (int Start, int Length)? field = null,
        NoteNumberFormat? format = null,
        PageFieldKind kind = PageFieldKind.PageNumber,
        (int Start, int Length)? second = null,
        PageFieldKind secondKind = PageFieldKind.PageCount)
    {
        List<PageRun> runs = [];
        if (field is { } span)
        {
            // Three runs so that the splice has something to rebuild: the number is coloured and its
            // neighbours are not, which is what a real "Page 3 of 9" footer looks like.
            runs.Add(new PageRun(0, span.Start, Face, Length.FromPoints(11)));
            runs.Add(new PageRun(
                span.Start, span.Length, Face, Length.FromPoints(11),
                Colour: Colour.FromRgb(0xC00000)));

            int tail = text.Length - (span.Start + span.Length);
            if (tail > 0)
            {
                runs.Add(new PageRun(span.Start + span.Length, tail, Face, Length.FromPoints(11)));
            }
        }

        return
        [
            new PageParagraph
            {
                Text = text,
                Face = Face,
                EmSize = Length.FromPoints(11),
                Runs = runs,
                Fields = field is { } at
                    ? second is { } also
                        ? [
                            new PageFieldSpan(at.Start, at.Length, kind, format),
                            new PageFieldSpan(also.Start, also.Length, secondKind, format),
                        ]
                        : [new PageFieldSpan(at.Start, at.Length, kind, format)]
                    : [],
            },
        ];
    }

    private static List<LaidOutPage> Paginate(
        IReadOnlyList<PageBlock> footer,
        NoteNumberFormat format = NoteNumberFormat.Arabic,
        int? restartAt = null)
    {
        PageFurnitureSet furniture = new(
            footers: new Dictionary<PageFurnitureSlot, IReadOnlyList<PageBlock>>
            {
                [PageFurnitureSlot.Default] = footer,
            });

        return new Paginator(PaginationOptions.Word).Paginate(
            Body(),
            new WritingSection
            {
                Page = Geometry,
                PageNumberFormat = format,
                RestartPageNumberAt = restartAt,
            },
            furniture: furniture);
    }

    /// <summary>Enough body to fill three pages of the geometry below.</summary>
    private static IReadOnlyList<PageBlock> Body()
        => [.. Enumerable.Range(0, 120).Select(i => new PageParagraph
        {
            Text = $"Body paragraph {i}",
            Face = Face,
            EmSize = Length.FromPoints(11),
        })];

    /// <summary>An A4 page with an inch of margin and the furniture half an inch into it.</summary>
    private static PageGeometry Geometry { get; } = new()
    {
        Size = new DocSize(Length.FromTwips(11906), Length.FromTwips(16838)),
        Margins = PageMargins.Uniform(Length.FromTwips(1440)),
        HeaderDistance = Length.FromTwips(720),
        FooterDistance = Length.FromTwips(720),
    };

    private static OpenTypeFace Face { get; } = Resolve();

    private static OpenTypeFace Resolve()
    {
        SystemFontResolver resolver = new(SystemFontIndex.Build());
        return resolver.LoadOpenType(
            resolver.Resolve(new FontRequest("Liberation Serif", 400, false)));
    }
}
