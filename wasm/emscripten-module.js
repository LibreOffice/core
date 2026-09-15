/* exported createEmscriptenModule */
function createEmscriptenModule(documentKind, documentDescriptor) {
	return {
		arguments: [documentKind, documentDescriptor],
	};
}
