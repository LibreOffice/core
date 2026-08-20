var commands = {
	insertWordCount: function () {
		var desktop = uno.idl.com.sun.star.frame.Desktop.create(uno.componentContext);
		var controller = desktop.getCurrentFrame().getController();
		var model = controller.getModel();
		var text = model.getText().getString().trim();
		var words = text.length ? text.split(/\s+/).length : 0;
		var viewCursor = controller.getViewCursor();
		model.getText().insertString(viewCursor, 'Word count: ' + words, false);
	},
};
