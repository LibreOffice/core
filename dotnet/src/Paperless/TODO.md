# Paperless (facade) — TODO

The one-line entry point: sniff a file and dispatch to the right reader.

- [ ] `PaperlessDocument.Identify` over `IFormatIdentifier`
- [ ] `PaperlessDocument.Open` dispatching by identified family
- [ ] `PaperlessDocument.Extract` as the shortest path to content
- [ ] Reader registry, so a consumer can add a reader without modifying this library
- [ ] Clear errors: an unsupported-but-recognised format must say *which* format it is, not
      just fail
- [ ] Encrypted documents: surface a `PasswordRequiredException` that distinguishes "no
      password given" from "password wrong"
- [ ] Async overloads — decide whether they are worth it given that readers seek freely over
      a buffered stream, so most of the work is CPU-bound rather than I/O-bound

## Open questions

- [ ] Should the facade expose per-family readers directly, or keep them behind it? Callers
      minimising dependencies want the sub-libraries; callers wanting simplicity want only
      this one. Currently both are public.
