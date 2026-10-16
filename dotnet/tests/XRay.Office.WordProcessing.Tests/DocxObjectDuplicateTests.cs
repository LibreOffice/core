using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A <c>w:object</c>'s replacement picture was drawn twice, because the filter meant to stop it
/// could never be false.
/// </summary>
/// <remarks>
/// <para>
/// <c>DocxVmlFrames.TopLevel</c> selected with <c>IsShape(child) is not false</c> against a
/// predicate that returned <c>true</c> or <c>null</c> and <b>never <c>false</c></b>, so the only
/// condition doing any work was the group test beside it: every descendant of the element was
/// offered to <c>One</c>. The class' own remark said <c>v:shapetype</c> "is excluded deliberately"
/// and described a filter that did not exist.
/// </para>
/// <para>
/// Inside a <c>w:pict</c> that costs nothing — <c>One</c> wants a size and a property element
/// states none. Inside a <c>w:object</c> it costs a duplicate of everything, through two
/// independent fallbacks: <c>One</c> takes the <em>object's</em> <c>w:dxaOrig</c>/<c>w:dyaOrig</c>
/// when the element states no box of its own, and that is the same pair for every descendant; and
/// <c>DocxPictures.ReadVml</c> searches <c>DescendantsAndSelf</c>, so the <c>v:imagedata</c>
/// element resolves <em>itself</em>.
/// </para>
/// <para>
/// The fixture is the smallest shape that shows it and it reproduces the two rectangles exactly:
/// <c>77.25 × 49.5</c> from the <c>v:shape</c>'s own <c>style</c>, and <c>77 × 49.85</c> from
/// <c>w:dxaOrig="1540"</c>/<c>w:dyaOrig="997"</c> over twenty. 26.2.4.2 draws <b>one</b> picture,
/// at <c>77.3 × 49.5</c>.
/// </para>
/// <para>
/// The predicate now names all ten real VML shape elements and answers false for the property
/// elements, which is what its remark always claimed. <strong>Only the answering-false half has
/// any reach.</strong> An earlier draft of this remark argued that narrowing to the original five
/// would drop 72 sized <c>v:line</c>; that figure was wrong — all <b>150</b> top-level
/// <c>v:line</c> in the corpus state <c>from</c>/<c>to</c> attributes, none states a
/// <c>style</c> width or height, and every one is <c>position:absolute</c>, so the floating path
/// returns null for each under either predicate. Rendering the <b>53</b> corpus documents holding
/// one of the five added elements with each predicate gives <b>53 byte-identical PDFs</b>.
/// </para>
/// <para>
/// Reach, measured by rendering the whole 337-document words track twice under
/// <c>SOURCE_DATE_EPOCH</c>: <b>8 renderings move, 329 byte-identical</b>, and <b>no page count
/// moves</b>. Five of the eight also lose characters and every one of them moves <em>towards</em>
/// 26.2.4.2 — this fixture's own witness <c>A1. EASA Form 2</c> 11 561 → 11 547 against the
/// reference's 11 527, <c>EHEST-SMS-Safety-Management-Manual-V2</c> 105 466 → 105 317 against
/// 105 180, and <c>UG.CAO.00006</c> 42 116 → 41 545 against 40 732 — so what went was drawn
/// twice. On that witness the image placements go <b>16 → 15</b> against the reference's
/// <b>15</b>.
/// </para>
/// </remarks>
public sealed class DocxObjectDuplicateTests
{
    private const string Fixture = "words-object-duplicate.docx";

    private static DrawnPage Page()
    {
        RecordingDrawingSink sink = new();

        using (DocumentSource source = DocumentSource.FromFile(Corpus.Require(Fixture)))
        {
            using IDocument document = new WordProcessingReader().Read(source);

            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            for (int i = 0; i < pages.Count; i++) pages[i].Draw(sink);
        }

        return sink.Pages[0];
    }

    [Fact]
    public void AnObjectsReplacementPictureIsDrawnOnce()
        => Page().Images.Count.ShouldBe(
            1, "the v:shape draws it; the v:imagedata inside it must not draw it again");

    [Fact]
    public void ItIsDrawnAtTheShapesOwnBoxRatherThanTheObjectsOriginalSize()
    {
        DocRect placed = Page().Images.Single();

        // The `v:shape` states 77.25pt x 49.5pt and the reference draws 77.3 x 49.5. The rejected
        // duplicate was 77 x 49.85 — `w:dxaOrig`/`w:dyaOrig` over twenty — so asserting the size
        // is what distinguishes "one picture" from "the wrong one of the two".
        placed.Width.Points.ShouldBe(77.25, 0.1);
        placed.Height.Points.ShouldBe(49.5, 0.1);
    }

    [Fact]
    public void TheSurroundingTextIsStillDrawn()
    {
        List<string> text = DrawnWords.On(Page()).Select(w => w.Text).ToList();

        // A filter that rejected too much would take the paragraphs with it.
        text.ShouldContain("Before.");
        text.ShouldContain("After.");
    }
}
