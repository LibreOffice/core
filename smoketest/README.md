Smoke test for each component of LibreOffice.

* smoketest:

	The main smoketest.cxx is launched connects via binary UNO
over a socket to a remote LibreOffice instance. This loads a document
which is zipped at build time into the workdir/ from the data/
directory. This in turn contains a set of macros in
data/Basic/Standard.

	Smoketest.cxx does a remote the StartTestWithDefaultOptions
macro and waits for a dispatchFinished from the macro's execution. To
debug this best load workdir/Zip/smoketestdoc.sxw - and hit 'start
smoketest' - this will launch a number of components and build a
suitable report in the form of a table.

	The StarBasic smoketests also log their output, this ends up
in instdir/user/temp/smoketest.log.
