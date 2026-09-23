// The text ranges that a command works on, which are the selected ones when there is a selection
// and the cursor otherwise:
function targetRanges() {
	var doc = cool.getActiveDocument();
	var selection = doc.getSelection();
	if (!selection) return [doc.getCursor().uno];
	var ranges = [];
	for (var i = 0; i < selection.uno.getCount(); ++i) ranges.push(selection.uno.getByIndex(i));
	return ranges;
}

function setParagraphStyle(name) {
	targetRanges().forEach(function (range) { range.setPropertyValue('ParaStyleName', name); });
}

var commands = {
	heading1: function () {
		setParagraphStyle('Heading 1');
	},
	heading2: function () {
		setParagraphStyle('Heading 2');
	},
	heading3: function () {
		setParagraphStyle('Heading 3');
	},
};
