var brandProductName = brandProductName === undefined ? 'Collabora Online' : brandProductName;
var brandProductURL = brandProductURL === undefined ? 'https://www.collaboraonline.com/' : brandProductURL;

var menuItems;
window.onload = function() {
	// wait until the menu (and particularly the document-header) actually exists
	function setLogo() {
		var logoHeader = document.getElementById('document-header');
		var logo = logoHeader && document.querySelector('#document-header > a');
		if (!logo) {
			// the logo does not exist in the menu yet, re-try in 250ms
			setTimeout(setLogo, 250);
		} else {
			logo.setAttribute('data-cooltip', brandProductName);
			logo.setAttribute('href', brandProductURL);
			logo.addEventListener('click', function(e) {
				// Route through window.open so the desktop apps reopen the link
				// in the system browser via the HYPERLINK bridge instead of a
				// new embedded webview window.
				e.preventDefault();
				window.open(brandProductURL, '_blank');
			});

			menuItems = document.querySelectorAll('#main-menu > li > a');
		}
	}
	function setAboutImg() {
		var lk = document.getElementById('lokit-version');
		var aboutDialog = document.getElementById('about-dialog-info');
		if (!lk || !aboutDialog) {
			setTimeout(setAboutImg, 250);
		} else {
			var div = document.createElement('div');
			div.style.marginInlineEnd = 'auto';
			div.id = 'lokit-extra';

			let span = document.createElement('span');
			span.setAttribute('dir', 'ltr');
			span.textContent = 'built on\u00A0';

			let anchor = document.createElement('a');
			anchor.href = 'https://col.la/lot';
			anchor.setAttribute('target', '_blank');
			anchor.textContent = 'a great technology base';

			div.appendChild(span);
			div.appendChild(anchor);
			lk.parentNode.parentNode.insertBefore(div, lk.parentNode);
		}
	}

	function addIntegratorSidebar() {
		var logoHeader = document.getElementById('document-header');
		if (!logoHeader) {
			// the logo does not exist in the menu yet, re-try in 250ms
			setTimeout(addIntegratorSidebar, 250);
		}
   }


	setLogo();
	setAboutImg();
	addIntegratorSidebar();
}

/*a::first-letter"*/
document.onkeyup = function(e) {
	if (e.altKey && e.shiftKey) {
		menuItems.forEach(function(menuItem) {
			menuItem.style.setProperty('text-decoration', 'underline', 'important');
		});
	}
};
