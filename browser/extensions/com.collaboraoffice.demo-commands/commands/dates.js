var commands = {
	insertDate: function () {
		var desktop = uno.idl.com.sun.star.frame.Desktop.create(uno.componentContext);
		var controller = desktop.getCurrentFrame().getController();
		var viewCursor = controller.getViewCursor();
		controller.getModel().getText().insertString(viewCursor, new Date().toLocaleDateString(), false);
	},
	insertTime: function () {
		var desktop = uno.idl.com.sun.star.frame.Desktop.create(uno.componentContext);
		var controller = desktop.getCurrentFrame().getController();
		var viewCursor = controller.getViewCursor();
		controller.getModel().getText().insertString(viewCursor, new Date().toLocaleTimeString(), false);
	},
	insertIsoDate: function () {
		var desktop = uno.idl.com.sun.star.frame.Desktop.create(uno.componentContext);
		var controller = desktop.getCurrentFrame().getController();
		var viewCursor = controller.getViewCursor();
		var iso = new Date().toISOString().slice(0, 10);
		controller.getModel().getText().insertString(viewCursor, iso, false);
	},
	insertLocaleDate: function () {
		var desktop = uno.idl.com.sun.star.frame.Desktop.create(uno.componentContext);
		var controller = desktop.getCurrentFrame().getController();
		var viewCursor = controller.getViewCursor();
		var formatted = new Date().toLocaleDateString(undefined, {
			weekday: 'long',
			year: 'numeric',
			month: 'long',
			day: 'numeric',
		});
		controller.getModel().getText().insertString(viewCursor, formatted, false);
	},
};
