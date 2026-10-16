using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Model;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A footnote that will not fit moves to the next page; the text citing it does not.
/// </summary>
/// <remarks>
/// <para>
/// The paginator used to shorten a paragraph's line count until the notes those lines cited fitted too,
/// so a note that would not fit removed the very line that cited it. That is what made
/// <c>template---tpr-technical-progress-report-with-guidance.docx</c> render 8 pages against 7: its page
/// 2 cites a footnote from the third line of a bullet, charging that footnote left no room for the
/// bullet, and the <c>Heading2</c> above it — <c>keepNext</c>, <c>keepLines</c> — followed it forward.
/// </para>
/// <para>
/// The reference draws the superscript on page 2 at y=659.1 and the note's own text on page 3 at
/// y=675.3. Our page 2 stopped at y=576.2 with the footnote at y=708.2 — 132 pt of empty body where the
/// reference leaves 33.
/// </para>
/// <para>
/// <strong>Measured before implementing, because two rules produce that output.</strong> Writer might
/// move the note whole or split it, and on that document they are indistinguishable — the note area was
/// already filled by an earlier footnote, so a split would have left nothing behind either. A probe
/// citing a 60-word note from a controlled body depth
/// (<c>probes/footnote-deferral/footnote-deferral.py</c>) leaves <strong>49</strong> of the note's 59
/// words on the citing page at one body length and <strong>17</strong> at the next. That is a cut at the
/// room remaining; a whole move predicts nought or fifty-nine and never seventeen.
/// </para>
/// <para>
/// What is implemented here is the boundary case of that rule — a note is carried whole rather than cut
/// at its last fitting line — because splitting needs a note to be layable from a line offset, which
/// nothing in the layouter can do yet. The corpus needs only the boundary: <c>tpr</c> closes at 7 pages
/// of 7 and <c>EHEST-SMS-Safety-Management-Manual-V2.docx</c> moves 80/82 to 81/82.
/// </para>
/// <para>
/// The probe's own remaining disagreement is recorded rather than papered over. We take <strong>13</strong>
/// pages there against the reference's 14, having taken 15 before, and the citing page agrees on six of
/// the ten cases outright. The three that still differ are exactly the ones the reference splits: we move
/// the note whole where it leaves 17 or 49 words behind. That is the other half of the work.
/// </para>
/// </remarks>
public sealed class NoteSpillTests
{
    /// <summary>The line citing a note stays even when the note cannot fit beneath it.</summary>
    /// <remarks>
    /// The note is made taller than any room a full page could leave, so there is no body length at
    /// which both could be placed together. Before the change the citing paragraph was pushed to the
    /// next page and the note went with it.
    /// </remarks>
    [Fact]
    public void ANoteThatCannotFitDoesNotTakeItsCitingLineWithIt()
    {
        List<LaidOutPage> pages = Paginate(bodyLines: 40, noteLines: 20);

        // The citing paragraph is the last body block, so it is on page one exactly when page one holds
        // every body line — which is the whole claim.
        pages[0].Lines.Count.ShouldBe(41, "40 body lines and the citing line, none pushed off");
    }

    /// <summary>And the note it could not take turns up on the following page.</summary>
    /// <remarks>
    /// Without this the first test would pass against an implementation that simply dropped the note,
    /// which is a worse defect than the one being fixed and would be invisible to a page count.
    /// </remarks>
    [Fact]
    public void TheNoteItCouldNotTakeIsDrawnOnTheNextPage()
    {
        List<LaidOutPage> pages = Paginate(bodyLines: 40, noteLines: 20);

        pages.Count.ShouldBeGreaterThan(1, "the note needs a page to land on");
        pages[0].Notes.ShouldBeNull("nothing of the note fitted beneath a full body");
        pages[1].Notes.ShouldNotBeNull("the note moved rather than being dropped");
        pages[1].Notes!.Lines.Count.ShouldBeGreaterThan(0);
    }

    /// <summary>A note that fits is still drawn on the page that cites it.</summary>
    /// <remarks>
    /// The guard on the change. Spilling is for the case where there is no room, and a rule that moved
    /// every note one page on would pass both tests above.
    /// </remarks>
    [Fact]
    public void ANoteThatFitsStaysOnItsCitingPage()
    {
        List<LaidOutPage> pages = Paginate(bodyLines: 3, noteLines: 1);

        pages[0].Notes.ShouldNotBeNull("there is ample room, so the note belongs here");
        pages[0].Notes!.Lines.Count.ShouldBe(1);
    }

    /// <summary>
    /// A note taller than the page it lands on is placed anyway rather than carried for ever.
    /// </summary>
    /// <remarks>
    /// The termination guard, and it is the same one the body flow uses for a paragraph too tall for a
    /// column of its own: the first note on a page is placed whatever the room. Without it a note longer
    /// than a page would be handed from page to page until the document ran out.
    /// </remarks>
    [Fact]
    public void ANoteTallerThanAPageIsPlacedRatherThanCarriedForEver()
    {
        List<LaidOutPage> pages = Paginate(bodyLines: 40, noteLines: 200);

        pages.Count.ShouldBeLessThan(20, "a carried-for-ever note would run the page count away");
        pages.Any(p => p.Notes is { Lines.Count: > 0 })
            .ShouldBeTrue("the oversized note is drawn somewhere");
    }

    private static List<LaidOutPage> Paginate(int bodyLines, int noteLines)
    {
        List<PageBlock> body =
            [.. Enumerable.Range(0, bodyLines).Select(i => Paragraph($"body line {i}"))];

        PageParagraph citing = Paragraph("cites") with
        {
            Notes =
            [
                new PageNote
                {
                    Blocks = [.. Enumerable.Range(0, noteLines).Select(i => Paragraph($"note line {i}"))],
                    Offset = 0,
                },
            ],
        };

        body.Add(citing);

        return new Paginator(PaginationOptions.Word).Paginate(
            body, new WritingSection { Page = Geometry });
    }

    private static PageParagraph Paragraph(string text) => new()
    {
        Text = text,
        Face = Face,
        EmSize = Length.FromPoints(11),
    };

    /// <summary>An A4 page with one inch margins, so the body holds about fifty lines at 11 pt.</summary>
    private static PageGeometry Geometry { get; } = new()
    {
        Size = new DocSize(Length.FromTwips(11906), Length.FromTwips(16838)),
        Margins = PageMargins.Uniform(Length.FromTwips(1440)),
    };

    private static OpenTypeFace Face { get; } = Resolve();

    private static OpenTypeFace Resolve()
    {
        SystemFontResolver resolver = new(SystemFontIndex.Build());
        return resolver.LoadOpenType(
            resolver.Resolve(new FontRequest("Liberation Serif", 400, false)));
    }
}
