using Markdig;
using Markdig.Syntax;
using Markdig.Syntax.Inlines;
using Paperless.Core.Extraction;
using Paperless.Markup;

namespace Paperless.Markup.Tests;

/// <summary>
/// Emitting Markdown and reading it back with a parser that is not ours.
/// </summary>
/// <remarks>
/// <para>
/// The round trip is the only test that can see an escaping bug. Emit a paragraph containing a
/// literal asterisk without escaping it and every character is still present in the output, so
/// a text diff reports a match while the asterisk has quietly turned the next word into
/// emphasis — or, worse, has eaten a table row. Parsing the result and asking what it
/// <em>says</em> is what catches that, and the parser has to be an independent implementation
/// or it would agree with our own mistakes.
/// </para>
/// <para>
/// Markdig is that implementation: BSD-2-Clause, pure managed, and a CommonMark 0.30 plus GFM
/// conformance-tested parser. It is referenced by this test project alone —
/// <c>Paperless.Markup</c> writes Markdown and never reads it, so nothing shipped gains a
/// dependency.
/// </para>
/// </remarks>
internal static class Markdown
{
    /// <summary>
    /// The pipeline the emitted Markdown is read with: GFM, which is the flavour the writer
    /// targets, and nothing beyond it.
    /// </summary>
    /// <remarks>
    /// Deliberately not <c>UseAdvancedExtensions</c>. That turns on syntax GitHub does not
    /// implement — abbreviations, citations, custom containers — and a test that passed because
    /// an exotic extension happened to parse our output would be measuring the wrong thing.
    /// </remarks>
    public static MarkdownPipeline Pipeline { get; } = new MarkdownPipelineBuilder()
        .UsePipeTables()
        .UseEmphasisExtras()
        .UseFootnotes()
        .Build();

    /// <summary>Parses Markdown into an abstract syntax tree.</summary>
    public static MarkdownDocument Parse(string markdown)
        => Markdig.Markdown.Parse(markdown, Pipeline);

    /// <summary>What the Markdown says, with every trace of its syntax removed.</summary>
    public static string PlainText(string markdown)
        => Markdig.Markdown.ToPlainText(markdown, Pipeline);

    /// <summary>Emits a content tree as Markdown and reads the result back as plain text.</summary>
    public static string RoundTrip(ContentNode content)
        => PlainText(MarkdownWriter.ToMarkdown(content));

    /// <summary>
    /// The words a parsed node says, ignoring how they were marked up.
    /// </summary>
    /// <remarks>
    /// Hand-walked rather than done with Markdig's <c>Descendants()</c>, because neither
    /// overload of that reaches the inlines of a <em>leaf</em> block: a block quote comes back
    /// full and a heading comes back empty, which looks precisely like the writer having emitted
    /// an empty heading.
    /// </remarks>
    public static string Text(MarkdownObject node)
    {
        System.Text.StringBuilder text = new();
        Walk(node);
        return text.ToString().Trim();

        void Walk(MarkdownObject current)
        {
            switch (current)
            {
                case LiteralInline literal:
                    text.Append(literal.Content.ToString()).Append(' ');
                    break;
                case LeafBlock { Inline: not null } leaf:
                    foreach (Inline inline in leaf.Inline) Walk(inline);
                    break;
                case ContainerInline container:
                    foreach (Inline inline in container) Walk(inline);
                    break;
                case ContainerBlock blocks:
                    foreach (Block block in blocks) Walk(block);
                    break;
                default:
                    break;
            }
        }
    }

    /// <summary>
    /// Collapses every run of whitespace to one space, for comparing what two texts say rather
    /// than how they were wrapped.
    /// </summary>
    public static string Flatten(string text)
        => string.Join(' ', text.Split((char[]?)null, StringSplitOptions.RemoveEmptyEntries));
}
