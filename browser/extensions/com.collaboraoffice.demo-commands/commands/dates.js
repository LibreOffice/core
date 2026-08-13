var commands = {
	insertDate: function () {
		var desktop = uno.idl.com.sun.star.frame.Desktop.create(uno.componentContext);
		var controller = desktop.getCurrentFrame().getController();
		var viewCursor = controller.getViewCursor();
		controller.getModel().getText().insertString(viewCursor, new Date().toLocaleDateString(), false);
	},
};
