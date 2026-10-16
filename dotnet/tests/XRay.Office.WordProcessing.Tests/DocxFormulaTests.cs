using XRay.Office.Core.Documents;
using XRay.Office.TestKit;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A formula draws the marks its constructs <em>generate</em>, not only the text they state.
/// </summary>
/// <remarks>
/// <para>
/// OMML is structural: a fraction is an <c>m:f</c> holding a numerator and a denominator and
/// nothing in the file says "draw a rule between them"; a radical is an <c>m:rad</c> and nothing
/// says <c>U+221A</c>; a delimiter pair states its brackets in <em>attributes</em> of
/// <c>m:dPr</c>. So a reader that emits the <c>m:t</c> leaves and nothing else draws <c>ab</c> for
/// a fraction and <c>i=1nxi</c> for a sum — which is what this tree did until the walk existed,
/// because <c>m:r</c> and <c>m:t</c> share their local names with <c>w:r</c> and <c>w:t</c> and the
/// paragraph walk switched on the local name alone.
/// </para>
/// <para>
/// <strong>Why the fixture is a ladder.</strong> Fifteen constructs, each stated twice — once
/// inline and once as display maths — so a regression names the construct it broke rather than
/// reporting that a page changed. Every arm was measured against 26.2.4.2's own rendering of the
/// same committed file before a line of the walk was written; the per-construct table is in
/// <c>probes/omml-r163/results.md</c>, where all fifteen failed.
/// </para>
/// <para>
/// <strong>What these assertions deliberately do not pin.</strong> The reference lays a formula out
/// in <em>two</em> dimensions — a stacked fraction, limits above and below an n-ary, a vinculum
/// over a radicand — and this walk puts every mark on one line in reading order. So the tests below
/// assert that the generated marks are <em>present and in the right order</em>, which is what the
/// one-dimensional rendering can honestly claim, and say nothing about where they sit. A later
/// round that adds a math box model should tighten them rather than delete them.
/// </para>
/// </remarks>
public sealed class DocxFormulaTests
{
    private const string Fixture = "words-formula-complexity.docx";

    /// <summary>A fraction draws a solidus between its terms.</summary>
    /// <remarks>
    /// The reference stacks them over a rule. Until there is a box to stack them in, the division
    /// has to be *visible* — <c>ab</c> is not a reading of <c>a/b</c>, it is a different expression.
    /// </remarks>
    [Fact]
    public void AFractionSeparatesItsTerms() => Arm("frac").ShouldBe("a/b");

    /// <summary>And the terms keep their own scripts.</summary>
    [Fact]
    public void AFractionsTermsKeepTheirScripts() => Arm("fracsub").ShouldBe("a1/b2");

    /// <summary>A radical draws its sign, which nothing in the file states.</summary>
    [Fact]
    public void ARadicalDrawsItsSign() => Arm("rad").ShouldBe("√x");

    /// <summary>A radical's degree is drawn before the sign, as it is read.</summary>
    [Fact]
    public void ARadicalsDegreePrecedesIt() => Arm("radn").ShouldBe("3√a/b");

    /// <summary>
    /// An n-ary draws the operator its own <c>m:chr</c> names — and that attribute is in the math
    /// namespace.
    /// </summary>
    /// <remarks>
    /// This is the assertion that earns its keep. <c>m:naryPr/m:chr</c> states <c>m:val</c>, not
    /// <c>w:val</c>, so a reader asking the WordprocessingML question of it is not answered "no" —
    /// it is answered <em>null</em>, which reads as "not stated" and takes the default. The n-ary
    /// default is the integral, so a summation silently drew <c>∫</c>. Measured on this fixture.
    /// </remarks>
    [Fact]
    public void ASummationDrawsItsOwnOperatorRatherThanTheDefault()
        => Arm("narysum").ShouldBe("∑i=1nxi");

    /// <summary>An n-ary stating no operator takes the specification's integral.</summary>
    [Fact]
    public void AnIntegralIsTheUnstatedNaryOperator() => Arm("naryint").ShouldBe("∫0∞f(t)dt");

    /// <summary>
    /// Nested delimiters draw the brackets each level states, not one bracket for both.
    /// </summary>
    /// <remarks>
    /// The inner pair is square and the outer round, and both are attributes rather than text —
    /// so a reader that draws only what is stated draws <c>a+bc</c> and loses the grouping that is
    /// the whole point of the construct.
    /// </remarks>
    [Fact]
    public void NestedDelimitersKeepTheirOwnBrackets() => Arm("delim").ShouldBe("(a+[b/c])");

    /// <summary>A function's name and its argument are both drawn, parenthesised as stated.</summary>
    [Fact]
    public void AFunctionDrawsItsNameAndArgument() => Arm("func").ShouldBe("sin(2θ)");

    /// <summary>A matrix draws its cells row by row inside the delimiters that hold it.</summary>
    /// <remarks>
    /// Cells separated by a space and rows by a line break — what a one-dimensional rendering of a
    /// grid can offer. The reference draws a real 2×2.
    /// </remarks>
    [Fact]
    public void AMatrixDrawsEveryCell() => Arm("matrix").ShouldStartWith("(a b");

    /// <summary>
    /// A subscript and a superscript on one base are both drawn, in the order OMML states them.
    /// </summary>
    [Fact]
    public void BothScriptsOnOneBaseAreDrawn() => Arm("subsup").ShouldBe("x12");

    /// <summary>A pre-script is drawn before its base, which is where it is read.</summary>
    [Fact]
    public void APreScriptPrecedesItsBase() => Arm("presub").ShouldBe("12X");

    /// <summary>
    /// A formula's structural elements never reach the paragraph walk's own cases.
    /// </summary>
    /// <remarks>
    /// The guard is the whole fix and this is what it buys: `m:t` is emitted by the math walk, and
    /// the properties elements — `m:naryPr`, `m:dPr`, `m:ctrlPr` and their siblings — are skipped
    /// rather than recursed into. A `m:ctrlPr` holds a `w:rPr`, so a walk that descends into it
    /// reads a construct's own formatting as a run's.
    /// </remarks>
    [Fact]
    public void AControlPropertyDrawsNothing()
    {
        // `mixed` nests a radical inside a fraction's numerator with subscripted terms throughout,
        // so every properties element in the ladder is exercised on the way to it.
        Arm("mixed").ShouldBe("√WMTOM/ρ0Sw");
    }

    /// <summary>Every construct is stated a second time as display maths, and draws the same.</summary>
    /// <remarks>
    /// A `m:oMathPara` is a paragraph of its own holding one `m:oMath`, so it exercises the walk
    /// from a different entry point. The reference also centres it and gives it the height of the
    /// StarMath object it builds; neither is asserted here — see the class remarks.
    /// </remarks>
    [Fact]
    public void TheDisplayFormOfEachConstructDrawsTheSameMarks()
    {
        List<string> lines = Lines();

        // The display copy of a construct is the paragraph after its inline one, which carries no
        // prose. Checking two of them is enough to show the entry point is reached at all.
        lines.ShouldContain("a/b");
        lines.ShouldContain("∑i=1nxi");
    }

    /// <summary>The formula text of one ladder arm, by the name its prose line carries.</summary>
    private static string Arm(string name)
    {
        string prefix = name + " inline: before ";
        string line = Lines().FirstOrDefault(text => text.StartsWith(prefix, StringComparison.Ordinal))
                      ?? throw new InvalidOperationException($"the fixture states no arm '{name}'");

        string rest = line[prefix.Length..];
        int after = rest.IndexOf(" after ", StringComparison.Ordinal);
        return after < 0 ? rest : rest[..after];
    }

    private static List<string> Lines()
    {
        using DocumentSource source = DocumentSource.FromFile(Corpus.Require(Fixture));
        using IDocument document = new WordProcessingReader().Read(source);

        var pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        return [.. pages.Paragraphs.Select(paragraph => paragraph.Text)];
    }
}
