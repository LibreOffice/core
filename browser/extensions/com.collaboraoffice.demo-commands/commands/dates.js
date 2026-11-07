// Insert text the way typing does, replacing the selection when there is one:
function insertText(text) {
	var doc = DocumentApp.getActiveDocument();
	var selection = doc.getSelection();
	if (selection) selection.replace(text);
	else doc.getCursor().insertText(text);
}

remote.insertDate = function insertDate() {
	insertText(new Date().toLocaleDateString());
};

remote.insertTime = function insertTime() {
	insertText(new Date().toLocaleTimeString());
};

remote.insertIsoDate = function insertIsoDate() {
	insertText(new Date().toISOString().slice(0, 10));
};

remote.insertLocaleDate = function insertLocaleDate() {
	insertText(new Date().toLocaleDateString(undefined, {
		weekday: 'long',
		year: 'numeric',
		month: 'long',
		day: 'numeric',
	}));
};
