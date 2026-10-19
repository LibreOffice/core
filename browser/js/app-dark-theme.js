/* -*- js-indent-level: 8; fill-column: 100 -*- */
// Apply dark theme immediately if darkTheme query parameter is present
(function () {
	const params = new URLSearchParams(window.location.search);
	if (params.get('darkTheme') === 'true') {
		document.documentElement.setAttribute('data-theme', 'dark');
		const link = document.createElement('link');
		link.setAttribute('rel', 'stylesheet');
		link.setAttribute('href', 'color-palette-dark.css');
		document.head.appendChild(link);
	}
})();
