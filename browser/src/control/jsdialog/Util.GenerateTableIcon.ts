/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

function parseHexToRgb(hex: string): [number, number, number] {
	const color = parseInt(hex.replace('#', ''), 16);
	return [(color & 0xff0000) >> 16, (color & 0x00ff00) >> 8, color & 0x0000ff];
}

function getContrastStroke(color: string): string {
	const [r, g, b] = parseHexToRgb(color);
	const brightness = (r * 299 + g * 587 + b * 114) / 1000;
	return brightness < 128 ? 'white' : 'black';
}

function getBodyContrastStroke(color1: string, color2: string): string {
	const brightness = (c: string) => {
		const [r, g, b] = parseHexToRgb(c);
		return (r * 299 + g * 587 + b * 114) / 1000;
	};
	const avg = (brightness(color1) + brightness(color2)) / 2;
	return avg < 128 ? 'white' : 'black';
}

function strengthenColor(hex: string, factor: number): string {
	const [r, g, b] = parseHexToRgb(hex);

	const rNorm = r / 255,
		gNorm = g / 255,
		bNorm = b / 255;
	const max = Math.max(rNorm, gNorm, bNorm);
	const min = Math.min(rNorm, gNorm, bNorm);
	const delta = max - min;

	let h = 0,
		s = 0,
		l = (max + min) / 2;

	if (delta !== 0) {
		s = delta / (1 - Math.abs(2 * l - 1));
		if (max === rNorm) h = ((gNorm - bNorm) / delta) % 6;
		else if (max === gNorm) h = (bNorm - rNorm) / delta + 2;
		else h = (rNorm - gNorm) / delta + 4;
		h = h * 60;
		if (h < 0) h += 360;
	}

	if (s < 0.15) {
		return l > 0.5 ? darkenColor(hex, 0.4) : darkenColor(hex, 0.6);
	}

	l = l * factor;

	const c = 1 - Math.abs(2 * l - 1);
	const x = c * (1 - Math.abs(((h / 60) % 2) - 1));
	const m = l - c / 2;

	let rOut = 0,
		gOut = 0,
		bOut = 0;
	if (h < 60) {
		rOut = c;
		gOut = x;
		bOut = 0;
	} else if (h < 120) {
		rOut = x;
		gOut = c;
		bOut = 0;
	} else if (h < 180) {
		rOut = 0;
		gOut = c;
		bOut = x;
	} else if (h < 240) {
		rOut = 0;
		gOut = x;
		bOut = c;
	} else if (h < 300) {
		rOut = x;
		gOut = 0;
		bOut = c;
	} else {
		rOut = c;
		gOut = 0;
		bOut = x;
	}

	return toHex((rOut + m) * 255, (gOut + m) * 255, (bOut + m) * 255);
}

function lightTableStyleSvg(
	headerColor: string,
	bodyColor: string,
	stripeColor: string,
	lightStyleIndex: number,
): string {
	const accentColor = strengthenColor(stripeColor, 0.25);

	// Light 1 - 7: Has alternating row stripes, top border and bottom border
	if (lightStyleIndex >= 1 && lightStyleIndex <= 7) {
		return `<svg viewBox="0 0 32 32" fill="none" xmlns="http://www.w3.org/2000/svg">

			<!-- Background -->
			<rect id="table-background" x="1.5" y="3.35" width="29" height="25.1" fill="${bodyColor}"/>

			<!-- Top border -->
			<line id="top-border" x1="1.5" y1="3.34998" x2="30.4942" y2="3.34998" stroke="${accentColor}" stroke-width="0.5"/>

			<!-- Header -->
			<g id="header-row">
				<rect id="header-bg" x="1.5" y="3.5" width="29" height="5" fill="${bodyColor}"/>
				<line id="header-column-5" x1="26.1" y1="6.18884" x2="29.1" y2="6.18884" stroke="${accentColor}" stroke-width="0.5"/>
				<line id="header-column-4" x1="20.3" y1="6.18884" x2="23.3" y2="6.18884" stroke="${accentColor}" stroke-width="0.5"/>
				<line id="header-column-3" x1="14.5" y1="6.18884" x2="17.5" y2="6.18884" stroke="${accentColor}" stroke-width="0.5"/>
				<line id="header-column-2" x1="8.7" y1="6.18884" x2="11.7" y2="6.18884" stroke="${accentColor}" stroke-width="0.5"/>
				<line id="header-column-1" x1="2.9" y1="6.18884" x2="5.9" y2="6.18884" stroke="${accentColor}" stroke-width="0.5"/>
			</g>

			<!-- Header separator -->
			<line id="header-separator" x1="1.5" y1="8.75" x2="30.5" y2="8.75" stroke="${accentColor}" stroke-width="0.5"/>

			<!-- Body rows -->
			<g id="body-rows">
				<g id="row-1">
					<rect id="row-1-bg" x="1.5" y="9" width="29" height="5" fill="${stripeColor}"/>
					<line id="row-1-column-5" x1="26.1" y1="11.0564" x2="29.1" y2="11.0564" stroke="${accentColor}" stroke-width="0.5"/>
					<line id="row-1-column-4" x1="20.3" y1="11.0564" x2="23.3" y2="11.0564" stroke="${accentColor}" stroke-width="0.5"/>
					<line id="row-1-column-3" x1="14.5" y1="11.0564" x2="17.5" y2="11.0564" stroke="${accentColor}" stroke-width="0.5"/>
					<line id="row-1-column-2" x1="8.7" y1="11.0564" x2="11.7" y2="11.0564" stroke="${accentColor}" stroke-width="0.5"/>
					<line id="row-1-column-1" x1="2.9" y1="11.0564" x2="5.9" y2="11.0564" stroke="${accentColor}" stroke-width="0.5"/>
				</g>

				<g id="row-2">
					<rect id="row-2-bg" x="1.5" y="13.5" width="29" height="5" fill="${bodyColor}"/>
					<line id="row-2-column-5" x1="26.1" y1="15.8564" x2="29.1" y2="15.8564" stroke="${accentColor}" stroke-width="0.5"/>
					<line id="row-2-column-4" x1="20.3" y1="15.8564" x2="23.3" y2="15.8564" stroke="${accentColor}" stroke-width="0.5"/>
					<line id="row-2-column-3" x1="14.5" y1="15.8564" x2="17.5" y2="15.8564" stroke="${accentColor}" stroke-width="0.5"/>
					<line id="row-2-column-2" x1="8.7" y1="15.8564" x2="11.7" y2="15.8564" stroke="${accentColor}" stroke-width="0.5"/>
					<line id="row-2-column-1" x1="2.9" y1="15.8564" x2="5.9" y2="15.8564" stroke="${accentColor}" stroke-width="0.5"/>
				</g>

				<g id="row-3">
					<rect id="row-3-bg" x="1.5" y="18.4" width="29" height="5" fill="${stripeColor}"/>
					<line id="row-3-column-5" x1="26.1" y1="20.7563" x2="29.1" y2="20.7563" stroke="${accentColor}" stroke-width="0.5"/>
					<line id="row-3-column-4" x1="20.3" y1="20.7563" x2="23.3" y2="20.7563" stroke="${accentColor}" stroke-width="0.5"/>
					<line id="row-3-column-3" x1="14.5" y1="20.7563" x2="17.5" y2="20.7563" stroke="${accentColor}" stroke-width="0.5"/>
					<line id="row-3-column-2" x1="8.7" y1="20.7563" x2="11.7" y2="20.7563" stroke="${accentColor}" stroke-width="0.5"/>
					<line id="row-3-column-1" x1="2.9" y1="20.7563" x2="5.9" y2="20.7563" stroke="${accentColor}" stroke-width="0.5"/>
				</g>

				<g id="row-4">
					<rect id="row-4-bg" x="1.5" y="23.3" width="29" height="5" fill="${bodyColor}"/>
					<line id="row-4-column-5" x1="26.1" y1="25.8051" x2="29.1" y2="25.8051" stroke="${accentColor}" stroke-width="0.5"/>
					<line id="row-4-column-4" x1="20.3" y1="25.8051" x2="23.3" y2="25.8051" stroke="${accentColor}" stroke-width="0.5"/>
					<line id="row-4-column-3" x1="14.5" y1="25.8051" x2="17.5" y2="25.8051" stroke="${accentColor}" stroke-width="0.5"/>
					<line id="row-4-column-2" x1="8.7" y1="25.8051" x2="11.7" y2="25.8051" stroke="${accentColor}" stroke-width="0.5"/>
					<line id="row-4-column-1" x1="2.9" y1="25.8051" x2="5.9" y2="25.8051" stroke="${accentColor}" stroke-width="0.5"/>
				</g>
			</g>

			<!-- Bottom border -->
			<line id="bottom-border" x1="1.5" y1="28.45" x2="30.4942" y2="28.45" stroke="${accentColor}" stroke-width="0.5"/>

		</svg>`;
	}

	// Light 8 - 14: Has horizontal separators and table border
	if (lightStyleIndex >= 8 && lightStyleIndex <= 14) {
		return `<svg width="32" height="32" viewBox="0 0 32 32" fill="none" xmlns="http://www.w3.org/2000/svg">
			<!-- Background -->
			<rect id="table-background" x="1.7" y="3.2" width="28.6" height="25.6" fill="#FFFF"/>

			<!-- Header -->
			<g id="header-row">
				<rect id="header-bg" x="1.5" y="3.29999" width="29" height="5.5" fill="${headerColor}"/>
				<line id="header-column-5" x1="26.1" y1="6.1438" x2="29.1" y2="6.1438" stroke="white" stroke-width="0.5"/>
				<line id="header-column-4" x1="20.3" y1="6.1438" x2="23.3" y2="6.1438" stroke="white" stroke-width="0.5"/>
				<line id="header-column-3" x1="14.5" y1="6.1438" x2="17.5" y2="6.1438" stroke="white" stroke-width="0.5"/>
				<line id="header-column-2" x1="8.7" y1="6.1438" x2="11.7" y2="6.1438" stroke="white" stroke-width="0.5"/>
				<line id="header-column-1" x1="2.9" y1="6.1438" x2="5.9" y2="6.1438" stroke="white" stroke-width="0.5"/>
			</g>

			<!-- Body -->
			<g id="body-rows">
				<!-- Row 1 -->
				<g id="row-1">
					<mask id="row-1-mask" fill="white">
						<path d="M1.5 8.70001H30.5V13.7H1.5V8.70001Z"/>
					</mask>
					<path id="row-1-divider" d="M30.5 13.7V13.2H1.5V13.7V14.2H30.5V13.7Z" fill="${headerColor}" mask="url(#row-1-mask)"/>
					<line id="row-1-column-5" x1="26.1" y1="11.0564" x2="29.1" y2="11.0564" stroke="black" stroke-width="0.5"/>
					<line id="row-1-column-4" x1="20.3" y1="11.0564" x2="23.3" y2="11.0564" stroke="black" stroke-width="0.5"/>
					<line id="row-1-column-3" x1="14.5" y1="11.0564" x2="17.5" y2="11.0564" stroke="black" stroke-width="0.5"/>
					<line id="row-1-column-2" x1="8.7" y1="11.0564" x2="11.7" y2="11.0564" stroke="black" stroke-width="0.5"/>
					<line id="row-1-column-1" x1="2.9" y1="11.0564" x2="5.9" y2="11.0564" stroke="black" stroke-width="0.5"/>
				</g>

				<!-- Row 2 -->
				<g id="row-2">
					<mask id="row-2-mask" fill="white">
						<path d="M1.5 13.5H30.5V18.5H1.5V13.5Z"/>
					</mask>
					<path id="row-2-divider" d="M30.5 18.5V18H1.5V18.5V19H30.5V18.5Z" fill="${headerColor}" mask="url(#row-2-mask)"/>
					<line id="row-2-column-5" x1="26.1" y1="15.8564" x2="29.1" y2="15.8564" stroke="black" stroke-width="0.5"/>
					<line id="row-2-column-4" x1="20.3" y1="15.8564" x2="23.3" y2="15.8564" stroke="black" stroke-width="0.5"/>
					<line id="row-2-column-3" x1="14.5" y1="15.8564" x2="17.5" y2="15.8564" stroke="black" stroke-width="0.5"/>
					<line id="row-2-column-2" x1="8.7" y1="15.8564" x2="11.7" y2="15.8564" stroke="black" stroke-width="0.5"/>
					<line id="row-2-column-1" x1="2.9" y1="15.8564" x2="5.9" y2="15.8564" stroke="black" stroke-width="0.5"/>
				</g>

				<!-- Row 3 -->
				<g id="row-3">
					<mask id="row-3-mask" fill="white">
						<path d="M1.5 18.4H30.5V23.4H1.5V18.4Z"/>
					</mask>
					<path id="row-3-divider" d="M30.5 23.4V22.9H1.5V23.4V23.9H30.5V23.4Z" fill="${headerColor}" mask="url(#row-3-mask)"/>
					<line id="row-3-column-5" x1="26.1" y1="20.7563" x2="29.1" y2="20.7563" stroke="black" stroke-width="0.5"/>
					<line id="row-3-column-4" x1="20.3" y1="20.7563" x2="23.3" y2="20.7563" stroke="black" stroke-width="0.5"/>
					<line id="row-3-column-3" x1="14.5" y1="20.7563" x2="17.5" y2="20.7563" stroke="black" stroke-width="0.5"/>
					<line id="row-3-column-2" x1="8.7" y1="20.7563" x2="11.7" y2="20.7563" stroke="black" stroke-width="0.5"/>
					<line id="row-3-column-1" x1="2.9" y1="20.7563" x2="5.9" y2="20.7563" stroke="black" stroke-width="0.5"/>
				</g>

				<!-- Row 4 -->
				<g id="row-4">
					<line id="row-4-column-5" x1="26.1" y1="25.8051" x2="29.1" y2="25.8051" stroke="black" stroke-width="0.5"/>
					<line id="row-4-column-4" x1="20.3" y1="25.8051" x2="23.3" y2="25.8051" stroke="black" stroke-width="0.5"/>
					<line id="row-4-column-3" x1="14.5" y1="25.8051" x2="17.5" y2="25.8051" stroke="black" stroke-width="0.5"/>
					<line id="row-4-column-2" x1="8.7" y1="25.8051" x2="11.7" y2="25.8051" stroke="black" stroke-width="0.5"/>
					<line id="row-4-column-1" x1="2.9" y1="25.8051" x2="5.9" y2="25.8051" stroke="black" stroke-width="0.5"/>
				</g>
			</g>

			<!-- Table border -->
			<rect id="table-border" x="1.7" y="3.2" width="28.6" height="25.6" stroke="${headerColor}" stroke-width="0.4"/>
		</svg>`;
	}

	// Light 15 - 21: Has vertical separators, header separator and table border
	return `<svg viewBox="0 0 32 32" fill="none" xmlns="http://www.w3.org/2000/svg">
		<!-- Background -->
		<rect id="table-background" x="1.7" y="3.2" width="28.6" height="25.6" fill="${bodyColor}"/>

		<!-- Header -->
		<g id="header-row">
			<rect id="header-bg" x="1.5" y="3.29999" width="29" height="5.5" fill="${bodyColor}"/>
			<line id="header-column-5" x1="26.1" y1="6.1438" x2="29.1" y2="6.1438" stroke="black" stroke-width="0.5"/>
			<line id="header-column-4" x1="20.3" y1="6.1438" x2="23.3" y2="6.1438" stroke="black" stroke-width="0.5"/>
			<line id="header-column-3" x1="14.5" y1="6.1438" x2="17.5" y2="6.1438" stroke="black" stroke-width="0.5"/>
			<line id="header-column-2" x1="8.7" y1="6.1438" x2="11.7" y2="6.1438" stroke="black" stroke-width="0.5"/>
			<line id="header-column-1" x1="2.9" y1="6.1438" x2="5.9" y2="6.1438" stroke="black" stroke-width="0.5"/>
		</g>

		<!-- Body rows -->
		<g id="body-rows">
			<g id="row-1">
				<path id="row-1-bg" d="M1.5 8.70001H30.5V13.7H1.5V8.70001Z" fill="${stripeColor}"/>
				<line id="row-1-column-5" x1="26.1" y1="11.0564" x2="29.1" y2="11.0564" stroke="black" stroke-width="0.5"/>
				<line id="row-1-column-4" x1="20.3" y1="11.0564" x2="23.3" y2="11.0564" stroke="black" stroke-width="0.5"/>
				<line id="row-1-column-3" x1="14.5" y1="11.0564" x2="17.5" y2="11.0564" stroke="black" stroke-width="0.5"/>
				<line id="row-1-column-2" x1="8.7" y1="11.0564" x2="11.7" y2="11.0564" stroke="black" stroke-width="0.5"/>
				<line id="row-1-column-1" x1="2.9" y1="11.0564" x2="5.9" y2="11.0564" stroke="black" stroke-width="0.5"/>
			</g>

			<g id="row-2">
				<path id="row-2-bg" d="M1.5 13.5H30.5V18.5H1.5V13.5Z" fill="${bodyColor}"/>
				<line id="row-2-column-5" x1="26.1" y1="15.8564" x2="29.1" y2="15.8564" stroke="black" stroke-width="0.5"/>
				<line id="row-2-column-4" x1="20.3" y1="15.8564" x2="23.3" y2="15.8564" stroke="black" stroke-width="0.5"/>
				<line id="row-2-column-3" x1="14.5" y1="15.8564" x2="17.5" y2="15.8564" stroke="black" stroke-width="0.5"/>
				<line id="row-2-column-2" x1="8.7" y1="15.8564" x2="11.7" y2="15.8564" stroke="black" stroke-width="0.5"/>
				<line id="row-2-column-1" x1="2.9" y1="15.8564" x2="5.9" y2="15.8564" stroke="black" stroke-width="0.5"/>
			</g>

			<g id="row-3">
				<path id="row-3-bg" d="M1.5 18.4H30.5V23.4H1.5V18.4Z" fill="${stripeColor}"/>
				<line id="row-3-column-5" x1="26.1" y1="20.7563" x2="29.1" y2="20.7563" stroke="black" stroke-width="0.5"/>
				<line id="row-3-column-4" x1="20.3" y1="20.7563" x2="23.3" y2="20.7563" stroke="black" stroke-width="0.5"/>
				<line id="row-3-column-3" x1="14.5" y1="20.7563" x2="17.5" y2="20.7563" stroke="black" stroke-width="0.5"/>
				<line id="row-3-column-2" x1="8.7" y1="20.7563" x2="11.7" y2="20.7563" stroke="black" stroke-width="0.5"/>
				<line id="row-3-column-1" x1="2.9" y1="20.7563" x2="5.9" y2="20.7563" stroke="black" stroke-width="0.5"/>
			</g>

			<g id="row-4">
				<rect id="row-4-bg" x="1.5" y="23.3" width="29" height="5.70001" fill="${bodyColor}"/>
				<line id="row-4-column-5" x1="26.1" y1="26.1909" x2="29.1" y2="26.1909" stroke="black" stroke-width="0.5"/>
				<line id="row-4-column-4" x1="20.3" y1="26.1909" x2="23.3" y2="26.1909" stroke="black" stroke-width="0.5"/>
				<line id="row-4-column-3" x1="14.5" y1="26.1909" x2="17.5" y2="26.1909" stroke="black" stroke-width="0.5"/>
				<line id="row-4-column-2" x1="8.7" y1="26.1909" x2="11.7" y2="26.1909" stroke="black" stroke-width="0.5"/>
				<line id="row-4-column-1" x1="2.9" y1="26.1909" x2="5.9" y2="26.1909" stroke="black" stroke-width="0.5"/>
			</g>
		</g>

		<!-- Vertical separators -->
		<g id="vertical-separators">
			<line id="vertical-separator-2" x1="7.3" y1="3.2" x2="7.3" y2="28.8" stroke="${accentColor}" stroke-width="0.5"/>
			<line id="vertical-separator-3" x1="13.1" y1="3.2" x2="13.1" y2="28.8" stroke="${accentColor}" stroke-width="0.5"/>
			<line id="vertical-separator-4" x1="18.9" y1="3.2" x2="18.9" y2="28.8" stroke="${accentColor}" stroke-width="0.5"/>
			<line id="vertical-separator-5" x1="24.7" y1="3.2" x2="24.7" y2="28.8" stroke="${accentColor}" stroke-width="0.5"/>
		</g>

		<!-- Horizontal separator -->
		<g id="horizontal-separators">
			<line id="header-separator" x1="1.7" y1="8.75" x2="30.3" y2="8.75" stroke="${accentColor}" stroke-width="0.5"/>
		</g>

		<!-- Table border -->
		<rect id="table-border" x="1.7" y="3.2" width="28.6" height="25.6" stroke="${accentColor}" stroke-width="0.4"/>
	</svg>`;
}

function mediumTableStyleSvg(
	headerColor: string,
	bodyColor: string,
	stripeColor: string,
	mediumStyleIndex: number,
): string {
	const accentColor = strengthenColor(stripeColor, 0.45);

	// Medium 1 - 7: Has header background and alternating row stripes and table border
	if (mediumStyleIndex >= 1 && mediumStyleIndex <= 7) {
		return `<svg viewBox="0 0 32 32" fill="none" xmlns="http://www.w3.org/2000/svg">
  <!-- Background -->
  <rect id="table-background" x="1.7" y="3.2" width="28.6" height="25.6" fill="#FFFF"/>

  <!-- Header -->
  <g id="header-row">
    <rect id="header-bg" x="1.5" y="3.29999" width="29" height="5.5" fill="${headerColor}"/>
    <line id="header-column-5" x1="26.1" y1="6.1438" x2="29.1" y2="6.1438" stroke="white" stroke-width="0.5"/>
    <line id="header-column-4" x1="20.3" y1="6.1438" x2="23.3" y2="6.1438" stroke="white" stroke-width="0.5"/>
    <line id="header-column-3" x1="14.5" y1="6.1438" x2="17.5" y2="6.1438" stroke="white" stroke-width="0.5"/>
    <line id="header-column-2" x1="8.7" y1="6.1438" x2="11.7" y2="6.1438" stroke="white" stroke-width="0.5"/>
    <line id="header-column-1" x1="2.9" y1="6.1438" x2="5.9" y2="6.1438" stroke="white" stroke-width="0.5"/>
  </g>

  <!-- Body rows -->
  <g id="body-rows">
    <g id="row-1">
      <path id="row-1-bg" d="M1.5 8.70001H30.5V13.7H1.5V8.70001Z" fill="${stripeColor}"/>
      <line id="row-1-column-5" x1="26.1" y1="11.0564" x2="29.1" y2="11.0564" stroke="black" stroke-width="0.5"/>
      <line id="row-1-column-4" x1="20.3" y1="11.0564" x2="23.3" y2="11.0564" stroke="black" stroke-width="0.5"/>
      <line id="row-1-column-3" x1="14.5" y1="11.0564" x2="17.5" y2="11.0564" stroke="black" stroke-width="0.5"/>
      <line id="row-1-column-2" x1="8.7" y1="11.0564" x2="11.7" y2="11.0564" stroke="black" stroke-width="0.5"/>
      <line id="row-1-column-1" x1="2.9" y1="11.0564" x2="5.9" y2="11.0564" stroke="black" stroke-width="0.5"/>
    </g>

    <g id="row-2">
      <path id="row-2-bg" d="M1.5 13.6H30.5V18.6H1.5V13.6Z" fill="${bodyColor}"/>
      <line id="row-2-column-5" x1="26.1" y1="15.9564" x2="29.1" y2="15.9564" stroke="black" stroke-width="0.5"/>
      <line id="row-2-column-4" x1="20.3" y1="15.9564" x2="23.3" y2="15.9564" stroke="black" stroke-width="0.5"/>
      <line id="row-2-column-3" x1="14.5" y1="15.9564" x2="17.5" y2="15.9564" stroke="black" stroke-width="0.5"/>
      <line id="row-2-column-2" x1="8.7" y1="15.9564" x2="11.7" y2="15.9564" stroke="black" stroke-width="0.5"/>
      <line id="row-2-column-1" x1="2.9" y1="15.9564" x2="5.9" y2="15.9564" stroke="black" stroke-width="0.5"/>
    </g>

    <g id="row-3">
      <path id="row-3-bg" d="M1.5 18.4H30.5V23.6H1.5V18.4Z" fill="${stripeColor}"/>
      <line id="row-3-column-5" x1="26.1" y1="20.8606" x2="29.1" y2="20.8606" stroke="black" stroke-width="0.5"/>
      <line id="row-3-column-4" x1="20.3" y1="20.8606" x2="23.3" y2="20.8606" stroke="black" stroke-width="0.5"/>
      <line id="row-3-column-3" x1="14.5" y1="20.8606" x2="17.5" y2="20.8606" stroke="black" stroke-width="0.5"/>
      <line id="row-3-column-2" x1="8.7" y1="20.8606" x2="11.7" y2="20.8606" stroke="black" stroke-width="0.5"/>
      <line id="row-3-column-1" x1="2.9" y1="20.8606" x2="5.9" y2="20.8606" stroke="black" stroke-width="0.5"/>
    </g>

    <g id="row-4">
      <rect id="row-4-bg" x="1.5" y="23.3" width="29" height="5.5" fill="${bodyColor}"/>
      <line id="row-4-column-5" x1="26.1" y1="25.4505" x2="29.1" y2="25.4505" stroke="black" stroke-width="0.5"/>
      <line id="row-4-column-4" x1="20.3" y1="25.4505" x2="23.3" y2="25.4505" stroke="black" stroke-width="0.5"/>
      <line id="row-4-column-3" x1="14.5" y1="25.4505" x2="17.5" y2="25.4505" stroke="black" stroke-width="0.5"/>
      <line id="row-4-column-2" x1="8.7" y1="25.4505" x2="11.7" y2="25.4505" stroke="black" stroke-width="0.5"/>
      <line id="row-4-column-1" x1="2.9" y1="25.4505" x2="5.9" y2="25.4505" stroke="black" stroke-width="0.5"/>
    </g>
  </g>

  <!-- Table border -->
  <rect id="table-border" x="1.7" y="3.2" width="28.6" height="25.6" stroke="${accentColor}" stroke-width="0.4"/>
</svg>`;
	}

	// Medium 8 - 15: Has vertical separators and header separator
	if (mediumStyleIndex >= 8 && mediumStyleIndex <= 15) {
		return `<svg viewBox="0 0 32 32" fill="none" xmlns="http://www.w3.org/2000/svg">
  <!-- Background -->
  <rect id="table-background" x="1.7" y="3.2" width="28.6" height="25.6" fill="#FFFF"/>

  <!-- Header -->
  <g id="header-row">
    <rect id="header-bg" x="1.5" y="3.29999" width="29" height="5.5" fill="${headerColor}"/>
    <line id="header-column-5" x1="26.1" y1="6.1438" x2="29.1" y2="6.1438" stroke="white" stroke-width="0.5"/>
    <line id="header-column-4" x1="20.3" y1="6.1438" x2="23.3" y2="6.1438" stroke="white" stroke-width="0.5"/>
    <line id="header-column-3" x1="14.5" y1="6.1438" x2="17.5" y2="6.1438" stroke="white" stroke-width="0.5"/>
    <line id="header-column-2" x1="8.7" y1="6.1438" x2="11.7" y2="6.1438" stroke="white" stroke-width="0.5"/>
    <line id="header-column-1" x1="2.9" y1="6.1438" x2="5.9" y2="6.1438" stroke="white" stroke-width="0.5"/>
  </g>

  <!-- Body rows -->
  <g id="body-rows">
    <g id="row-1">
      <path id="row-1-bg" d="M1.5 8.70001H30.5V13.7H1.5V8.70001Z" fill="${stripeColor}"/>
      <line id="row-1-column-5" x1="26.1" y1="11.0564" x2="29.1" y2="11.0564" stroke="black" stroke-width="0.5"/>
      <line id="row-1-column-4" x1="20.3" y1="11.0564" x2="23.3" y2="11.0564" stroke="black" stroke-width="0.5"/>
      <line id="row-1-column-3" x1="14.5" y1="11.0564" x2="17.5" y2="11.0564" stroke="black" stroke-width="0.5"/>
      <line id="row-1-column-2" x1="8.7" y1="11.0564" x2="11.7" y2="11.0564" stroke="black" stroke-width="0.5"/>
      <line id="row-1-column-1" x1="2.9" y1="11.0564" x2="5.9" y2="11.0564" stroke="black" stroke-width="0.5"/>
    </g>

    <g id="row-2">
      <path id="row-2-bg" d="M1.5 13.5H30.5V18.5H1.5V13.5Z" fill="${bodyColor}"/>
      <line id="row-2-column-5" x1="26.1" y1="15.8564" x2="29.1" y2="15.8564" stroke="black" stroke-width="0.5"/>
      <line id="row-2-column-4" x1="20.3" y1="15.8564" x2="23.3" y2="15.8564" stroke="black" stroke-width="0.5"/>
      <line id="row-2-column-3" x1="14.5" y1="15.8564" x2="17.5" y2="15.8564" stroke="black" stroke-width="0.5"/>
      <line id="row-2-column-2" x1="8.7" y1="15.8564" x2="11.7" y2="15.8564" stroke="black" stroke-width="0.5"/>
      <line id="row-2-column-1" x1="2.9" y1="15.8564" x2="5.9" y2="15.8564" stroke="black" stroke-width="0.5"/>
    </g>

    <g id="row-3">
      <path id="row-3-bg" d="M1.5 18.4H30.5V23.4H1.5V18.4Z" fill="${stripeColor}"/>
      <line id="row-3-column-5" x1="26.1" y1="20.7563" x2="29.1" y2="20.7563" stroke="black" stroke-width="0.5"/>
      <line id="row-3-column-4" x1="20.3" y1="20.7563" x2="23.3" y2="20.7563" stroke="black" stroke-width="0.5"/>
      <line id="row-3-column-3" x1="14.5" y1="20.7563" x2="17.5" y2="20.7563" stroke="black" stroke-width="0.5"/>
      <line id="row-3-column-2" x1="8.7" y1="20.7563" x2="11.7" y2="20.7563" stroke="black" stroke-width="0.5"/>
      <line id="row-3-column-1" x1="2.9" y1="20.7563" x2="5.9" y2="20.7563" stroke="black" stroke-width="0.5"/>
    </g>

    <g id="row-4">
      <rect id="row-4-bg" x="1.5" y="23.3" width="29" height="5.70001" fill="${bodyColor}"/>
      <line id="row-4-column-5" x1="26.1" y1="26.1909" x2="29.1" y2="26.1909" stroke="black" stroke-width="0.5"/>
      <line id="row-4-column-4" x1="20.3" y1="26.1909" x2="23.3" y2="26.1909" stroke="black" stroke-width="0.5"/>
      <line id="row-4-column-3" x1="14.5" y1="26.1909" x2="17.5" y2="26.1909" stroke="black" stroke-width="0.5"/>
      <line id="row-4-column-2" x1="8.7" y1="26.1909" x2="11.7" y2="26.1909" stroke="black" stroke-width="0.5"/>
      <line id="row-4-column-1" x1="2.9" y1="26.1909" x2="5.9" y2="26.1909" stroke="black" stroke-width="0.5"/>
    </g>
  </g>

  <!-- Vertical separators -->
  <g id="vertical-separators">
    <line id="vertical-separator-2" x1="7.3" y1="3.2" x2="7.3" y2="28.8" stroke="white" stroke-width="0.5"/>
    <line id="vertical-separator-3" x1="13.1" y1="3.2" x2="13.1" y2="28.8" stroke="white" stroke-width="0.5"/>
    <line id="vertical-separator-4" x1="18.9" y1="3.2" x2="18.9" y2="28.8" stroke="white" stroke-width="0.5"/>
    <line id="vertical-separator-5" x1="24.7" y1="3.2" x2="24.7" y2="28.8" stroke="white" stroke-width="0.5"/>
  </g>

  <!-- Horizontal separator -->
  <g id="horizontal-separators">
    <line id="header-separator" x1="1.7" y1="8.75" x2="30.3" y2="8.75" stroke="white" stroke-width="0.5"/>
  </g>

  <!-- Table border -->
  <rect id="table-border" x="1.7" y="3.2" width="28.6" height="25.6" stroke="transparent" stroke-width="0.4"/>
</svg>`;
	}

	// Medium 16 - 21: Has top border, header separator and bottom border
	if (mediumStyleIndex >= 16 && mediumStyleIndex <= 21) {
		return `<svg viewBox="0 0 32 32" fill="none" xmlns="http://www.w3.org/2000/svg">

  <!-- Background -->
  <rect id="table-background" x="1.5" y="3.35" width="29" height="25.1" fill="#FFFF"/>

  <!-- Top border -->
  <line id="top-border" x1="1.5" y1="3.34998" x2="30.4942" y2="3.34998" stroke="black" stroke-width="0.5"/>

  <!-- Header -->
  <g id="header-row">
    <rect id="header-bg" x="1.5" y="3.5" width="29" height="5" fill="${headerColor}"/>
    <line id="header-column-5" x1="26.1" y1="6.18884" x2="29.1" y2="6.18884" stroke="white" stroke-width="0.5"/>
    <line id="header-column-4" x1="20.3" y1="6.18884" x2="23.3" y2="6.18884" stroke="white" stroke-width="0.5"/>
    <line id="header-column-3" x1="14.5" y1="6.18884" x2="17.5" y2="6.18884" stroke="white" stroke-width="0.5"/>
    <line id="header-column-2" x1="8.7" y1="6.18884" x2="11.7" y2="6.18884" stroke="white" stroke-width="0.5"/>
    <line id="header-column-1" x1="2.9" y1="6.18884" x2="5.9" y2="6.18884" stroke="white" stroke-width="0.5"/>
  </g>

  <!-- Header separator -->
  <line id="header-separator" x1="1.5" y1="8.75" x2="30.5" y2="8.75" stroke="black" stroke-width="0.5"/>

  <!-- Body rows -->
  <g id="body-rows">
    <g id="row-1">
      <rect id="row-1-bg" x="1.5" y="9" width="29" height="5" fill="${stripeColor}"/>
      <line id="row-1-column-5" x1="26.1" y1="11.0564" x2="29.1" y2="11.0564" stroke="black" stroke-width="0.5"/>
      <line id="row-1-column-4" x1="20.3" y1="11.0564" x2="23.3" y2="11.0564" stroke="black" stroke-width="0.5"/>
      <line id="row-1-column-3" x1="14.5" y1="11.0564" x2="17.5" y2="11.0564" stroke="black" stroke-width="0.5"/>
      <line id="row-1-column-2" x1="8.7" y1="11.0564" x2="11.7" y2="11.0564" stroke="black" stroke-width="0.5"/>
      <line id="row-1-column-1" x1="2.9" y1="11.0564" x2="5.9" y2="11.0564" stroke="black" stroke-width="0.5"/>
    </g>

    <g id="row-2">
      <rect id="row-2-bg" x="1.5" y="13.5" width="29" height="5" fill="${bodyColor}"/>
      <line id="row-2-column-5" x1="26.1" y1="15.8564" x2="29.1" y2="15.8564" stroke="black" stroke-width="0.5"/>
      <line id="row-2-column-4" x1="20.3" y1="15.8564" x2="23.3" y2="15.8564" stroke="black" stroke-width="0.5"/>
      <line id="row-2-column-3" x1="14.5" y1="15.8564" x2="17.5" y2="15.8564" stroke="black" stroke-width="0.5"/>
      <line id="row-2-column-2" x1="8.7" y1="15.8564" x2="11.7" y2="15.8564" stroke="black" stroke-width="0.5"/>
      <line id="row-2-column-1" x1="2.9" y1="15.8564" x2="5.9" y2="15.8564" stroke="black" stroke-width="0.5"/>
    </g>

    <g id="row-3">
      <rect id="row-3-bg" x="1.5" y="18.4" width="29" height="5" fill="${stripeColor}"/>
      <line id="row-3-column-5" x1="26.1" y1="20.7563" x2="29.1" y2="20.7563" stroke="black" stroke-width="0.5"/>
      <line id="row-3-column-4" x1="20.3" y1="20.7563" x2="23.3" y2="20.7563" stroke="black" stroke-width="0.5"/>
      <line id="row-3-column-3" x1="14.5" y1="20.7563" x2="17.5" y2="20.7563" stroke="black" stroke-width="0.5"/>
      <line id="row-3-column-2" x1="8.7" y1="20.7563" x2="11.7" y2="20.7563" stroke="black" stroke-width="0.5"/>
      <line id="row-3-column-1" x1="2.9" y1="20.7563" x2="5.9" y2="20.7563" stroke="black" stroke-width="0.5"/>
    </g>

    <g id="row-4">
      <rect id="row-4-bg" x="1.5" y="23.3" width="29" height="5" fill="${bodyColor}"/>
      <line id="row-4-column-5" x1="26.1" y1="25.8051" x2="29.1" y2="25.8051" stroke="black" stroke-width="0.5"/>
      <line id="row-4-column-4" x1="20.3" y1="25.8051" x2="23.3" y2="25.8051" stroke="black" stroke-width="0.5"/>
      <line id="row-4-column-3" x1="14.5" y1="25.8051" x2="17.5" y2="25.8051" stroke="black" stroke-width="0.5"/>
      <line id="row-4-column-2" x1="8.7" y1="25.8051" x2="11.7" y2="25.8051" stroke="black" stroke-width="0.5"/>
      <line id="row-4-column-1" x1="2.9" y1="25.8051" x2="5.9" y2="25.8051" stroke="black" stroke-width="0.5"/>
    </g>
  </g>

  <!-- Bottom border -->
  <line id="bottom-border" x1="1.5" y1="28.45" x2="30.4942" y2="28.45" stroke="black" stroke-width="0.5"/>

</svg>`;
	}

	// Medium 22 - 28: Has vertical separators and table border
	return `<svg viewBox="0 0 32 32" fill="none" xmlns="http://www.w3.org/2000/svg">
  <!-- Background -->
  <rect id="table-background" x="1.7" y="3.2" width="28.6" height="25.6" fill="#FFFF"/>

  <!-- Header -->
  <g id="header-row">
    <rect id="header-bg" x="1.5" y="3.29999" width="29" height="5.5" fill="${headerColor}"/>
    <line id="header-column-5" x1="26.1" y1="6.1438" x2="29.1" y2="6.1438" stroke="black" stroke-width="0.5"/>
    <line id="header-column-4" x1="20.3" y1="6.1438" x2="23.3" y2="6.1438" stroke="black" stroke-width="0.5"/>
    <line id="header-column-3" x1="14.5" y1="6.1438" x2="17.5" y2="6.1438" stroke="black" stroke-width="0.5"/>
    <line id="header-column-2" x1="8.7" y1="6.1438" x2="11.7" y2="6.1438" stroke="black" stroke-width="0.5"/>
    <line id="header-column-1" x1="2.9" y1="6.1438" x2="5.9" y2="6.1438" stroke="black" stroke-width="0.5"/>
  </g>

  <!-- Body rows -->
  <g id="body-rows">
    <g id="row-1">
      <path id="row-1-bg" d="M1.5 8.70001H30.5V13.7H1.5V8.70001Z" fill="${stripeColor}"/>
      <line id="row-1-column-5" x1="26.1" y1="11.0564" x2="29.1" y2="11.0564" stroke="black" stroke-width="0.5"/>
      <line id="row-1-column-4" x1="20.3" y1="11.0564" x2="23.3" y2="11.0564" stroke="black" stroke-width="0.5"/>
      <line id="row-1-column-3" x1="14.5" y1="11.0564" x2="17.5" y2="11.0564" stroke="black" stroke-width="0.5"/>
      <line id="row-1-column-2" x1="8.7" y1="11.0564" x2="11.7" y2="11.0564" stroke="black" stroke-width="0.5"/>
      <line id="row-1-column-1" x1="2.9" y1="11.0564" x2="5.9" y2="11.0564" stroke="black" stroke-width="0.5"/>
    </g>

    <g id="row-2">
      <path id="row-2-bg" d="M1.5 13.5H30.5V18.5H1.5V13.5Z" fill="${bodyColor}"/>
      <line id="row-2-column-5" x1="26.1" y1="15.8564" x2="29.1" y2="15.8564" stroke="black" stroke-width="0.5"/>
      <line id="row-2-column-4" x1="20.3" y1="15.8564" x2="23.3" y2="15.8564" stroke="black" stroke-width="0.5"/>
      <line id="row-2-column-3" x1="14.5" y1="15.8564" x2="17.5" y2="15.8564" stroke="black" stroke-width="0.5"/>
      <line id="row-2-column-2" x1="8.7" y1="15.8564" x2="11.7" y2="15.8564" stroke="black" stroke-width="0.5"/>
      <line id="row-2-column-1" x1="2.9" y1="15.8564" x2="5.9" y2="15.8564" stroke="black" stroke-width="0.5"/>
    </g>

    <g id="row-3">
      <path id="row-3-bg" d="M1.5 18.4H30.5V23.4H1.5V18.4Z" fill="${stripeColor}"/>
      <line id="row-3-column-5" x1="26.1" y1="20.7563" x2="29.1" y2="20.7563" stroke="black" stroke-width="0.5"/>
      <line id="row-3-column-4" x1="20.3" y1="20.7563" x2="23.3" y2="20.7563" stroke="black" stroke-width="0.5"/>
      <line id="row-3-column-3" x1="14.5" y1="20.7563" x2="17.5" y2="20.7563" stroke="black" stroke-width="0.5"/>
      <line id="row-3-column-2" x1="8.7" y1="20.7563" x2="11.7" y2="20.7563" stroke="black" stroke-width="0.5"/>
      <line id="row-3-column-1" x1="2.9" y1="20.7563" x2="5.9" y2="20.7563" stroke="black" stroke-width="0.5"/>
    </g>

    <g id="row-4">
      <rect id="row-4-bg" x="1.5" y="23.3" width="29" height="5.70001" fill="${bodyColor}"/>
      <line id="row-4-column-5" x1="26.1" y1="26.1909" x2="29.1" y2="26.1909" stroke="black" stroke-width="0.5"/>
      <line id="row-4-column-4" x1="20.3" y1="26.1909" x2="23.3" y2="26.1909" stroke="black" stroke-width="0.5"/>
      <line id="row-4-column-3" x1="14.5" y1="26.1909" x2="17.5" y2="26.1909" stroke="black" stroke-width="0.5"/>
      <line id="row-4-column-2" x1="8.7" y1="26.1909" x2="11.7" y2="26.1909" stroke="black" stroke-width="0.5"/>
      <line id="row-4-column-1" x1="2.9" y1="26.1909" x2="5.9" y2="26.1909" stroke="black" stroke-width="0.5"/>
    </g>
  </g>

  <!-- Vertical separators -->
  <g id="vertical-separators">
    <line id="vertical-separator-2" x1="7.3" y1="3.2" x2="7.3" y2="28.8" stroke="${accentColor}" stroke-width="0.5"/>
    <line id="vertical-separator-3" x1="13.1" y1="3.2" x2="13.1" y2="28.8" stroke="${accentColor}" stroke-width="0.5"/>
    <line id="vertical-separator-4" x1="18.9" y1="3.2" x2="18.9" y2="28.8" stroke="${accentColor}" stroke-width="0.5"/>
    <line id="vertical-separator-5" x1="24.7" y1="3.2" x2="24.7" y2="28.8" stroke="${accentColor}" stroke-width="0.5"/>
  </g>

  <!-- Table border -->
  <rect id="table-border" x="1.7" y="3.2" width="28.6" height="25.6" stroke="${accentColor}" stroke-width="0.4"/>
</svg>`;
}

function darkTableStyleSvg(
	headerColor: string,
	bodyColor: string,
	firstRowStripe: string,
	styleIndex: number,
): string {
	const headerLineColor =
		styleIndex >= 1 && styleIndex <= 11 ? 'white' : 'black';
	const bodyLineColor = styleIndex >= 1 && styleIndex <= 7 ? 'white' : 'black';
	const separatorColor =
		styleIndex >= 1 && styleIndex <= 7 ? '#FFFF' : firstRowStripe;

	return `<svg viewBox="0 0 32 32" xmlns="http://www.w3.org/2000/svg">

		<!-- Background -->
		<rect id="table-background" x="1.5" y="2.95" width="29" height="25.35" fill="#FFFF"/>

		<!-- Header -->
		<rect x="1.5" y="2.95" width="29" height="5" fill="${headerColor}"/>
		<line id="header-column-5" x1="26.1" y1="5.51251" x2="29.1" y2="5.51251" stroke="${headerLineColor}" stroke-width="0.5"/>
		<line id="header-column-4" x1="20.3" y1="5.51251" x2="23.3" y2="5.51251" stroke="${headerLineColor}" stroke-width="0.5"/>
		<line id="header-column-3" x1="14.5" y1="5.51251" x2="17.5" y2="5.51251" stroke="${headerLineColor}" stroke-width="0.5"/>
		<line id="header-column-2" x1="8.7" y1="5.51251" x2="11.7" y2="5.51251" stroke="${headerLineColor}" stroke-width="0.5"/>
		<line id="header-column-1" x1="2.9" y1="5.51251" x2="5.9" y2="5.51251" stroke="${headerLineColor}" stroke-width="0.5"/>

		<!-- Row 1 -->
		<rect id="row-1-bg" x="1.5" y="8.70001" width="29" height="5" fill="${firstRowStripe}"/>
		<line id="row-1-column-5" x1="26.1" y1="11.0564" x2="29.1" y2="11.0564" stroke="${bodyLineColor}" stroke-width="0.5"/>
		<line id="row-1-column-4" x1="20.3" y1="11.0564" x2="23.3" y2="11.0564" stroke="${bodyLineColor}" stroke-width="0.5"/>
		<line id="row-1-column-3" x1="14.5" y1="11.0564" x2="17.5" y2="11.0564" stroke="${bodyLineColor}" stroke-width="0.5"/>
		<line id="row-1-column-2" x1="8.7" y1="11.0564" x2="11.7" y2="11.0564" stroke="${bodyLineColor}" stroke-width="0.5"/>
		<line id="row-1-column-1" x1="2.9" y1="11.0564" x2="5.9" y2="11.0564" stroke="${bodyLineColor}" stroke-width="0.5"/>

		<!-- Row 2 -->
		<rect id="row-2-bg" x="1.5" y="13.5" width="29" height="5" fill="${bodyColor}"/>
		<line id="row-2-column-5" x1="26.1" y1="15.8564" x2="29.1" y2="15.8564" stroke="${bodyLineColor}" stroke-width="0.5"/>
		<line id="row-2-column-4" x1="20.3" y1="15.8564" x2="23.3" y2="15.8564" stroke="${bodyLineColor}" stroke-width="0.5"/>
		<line id="row-2-column-3" x1="14.5" y1="15.8564" x2="17.5" y2="15.8564" stroke="${bodyLineColor}" stroke-width="0.5"/>
		<line id="row-2-column-2" x1="8.7" y1="15.8564" x2="11.7" y2="15.8564" stroke="${bodyLineColor}" stroke-width="0.5"/>
		<line id="row-2-column-1" x1="2.9" y1="15.8564" x2="5.9" y2="15.8564" stroke="${bodyLineColor}" stroke-width="0.5"/>

		<!-- Row 3 -->
		<rect id="row-3-bg" x="1.5" y="18.4" width="29" height="5" fill="${firstRowStripe}"/>
		<line id="row-3-column-5" x1="26.1" y1="20.7563" x2="29.1" y2="20.7563" stroke="${bodyLineColor}" stroke-width="0.5"/>
		<line id="row-3-column-4" x1="20.3" y1="20.7563" x2="23.3" y2="20.7563" stroke="${bodyLineColor}" stroke-width="0.5"/>
		<line id="row-3-column-3" x1="14.5" y1="20.7563" x2="17.5" y2="20.7563" stroke="${bodyLineColor}" stroke-width="0.5"/>
		<line id="row-3-column-2" x1="8.7" y1="20.7563" x2="11.7" y2="20.7563" stroke="${bodyLineColor}" stroke-width="0.5"/>
		<line id="row-3-column-1" x1="2.9" y1="20.7563" x2="5.9" y2="20.7563" stroke="${bodyLineColor}" stroke-width="0.5"/>

		<!-- Row 4 -->
		<rect id="row-4-bg" x="1.5" y="23.3" width="29" height="5" fill="${bodyColor}"/>
		<line id="row-4-column-5" x1="26.1" y1="25.8051" x2="29.1" y2="25.8051" stroke="${bodyLineColor}" stroke-width="0.5"/>
		<line id="row-4-column-4" x1="20.3" y1="25.8051" x2="23.3" y2="25.8051" stroke="${bodyLineColor}" stroke-width="0.5"/>
		<line id="row-4-column-3" x1="14.5" y1="25.8051" x2="17.5" y2="25.8051" stroke="${bodyLineColor}" stroke-width="0.5"/>
		<line id="row-4-column-2" x1="8.7" y1="25.8051" x2="11.7" y2="25.8051" stroke="${bodyLineColor}" stroke-width="0.5"/>
		<line id="row-4-column-1" x1="2.9" y1="25.8051" x2="5.9" y2="25.8051" stroke="${bodyLineColor}" stroke-width="0.5"/>

		<!-- Header separator -->
		<line id="header-separator" x1="1.5" y1="8.5" x2="30.5" y2="8.5" stroke="${separatorColor}"/>

	</svg>`;
}

function customTableStyleSvg(
	headerColor: string,
	firstRowStripe: string,
	secondRowStripe: string,
): string {
	const lineColor = getBodyContrastStroke(firstRowStripe, secondRowStripe);

	return `<svg viewBox="0 0 32 32" fill="none" xmlns="http://www.w3.org/2000/svg">

		<!-- Header -->
		<rect x="1.5" y="3" width="29" height="5.5" fill="${headerColor}"/>
		<line x1="26.1" y1="5.8" x2="29.1" y2="5.8" stroke="${getContrastStroke(headerColor)}" stroke-width="0.5"/>
		<line x1="20.3" y1="5.8" x2="23.3" y2="5.8" stroke="${getContrastStroke(headerColor)}" stroke-width="0.5"/>
		<line x1="14.5" y1="5.8" x2="17.5" y2="5.8" stroke="${getContrastStroke(headerColor)}" stroke-width="0.5"/>
		<line x1="8.7" y1="5.8" x2="11.7" y2="5.8" stroke="${getContrastStroke(headerColor)}" stroke-width="0.5"/>
		<line x1="2.9" y1="5.8" x2="5.9" y2="5.8" stroke="${getContrastStroke(headerColor)}" stroke-width="0.5"/>

		<!-- Row 1 -->
		<rect x="1.5" y="8.7" width="29" height="5" fill="${firstRowStripe}"/>
		<line x1="26.1" y1="11.1" x2="29.1" y2="11.1" stroke="${lineColor}" stroke-width="0.5"/>
		<line x1="20.3" y1="11.1" x2="23.3" y2="11.1" stroke="${lineColor}" stroke-width="0.5"/>
		<line x1="14.5" y1="11.1" x2="17.5" y2="11.1" stroke="${lineColor}" stroke-width="0.5"/>
		<line x1="8.7" y1="11.1" x2="11.7" y2="11.1" stroke="${lineColor}" stroke-width="0.5"/>
		<line x1="2.9" y1="11.1" x2="5.9" y2="11.1" stroke="${lineColor}" stroke-width="0.5"/>

		<!-- Row 2 -->
		<rect x="1.5" y="13.5" width="29" height="5" fill="${secondRowStripe}"/>
		<line x1="26.1" y1="15.9" x2="29.1" y2="15.9" stroke="${lineColor}" stroke-width="0.5"/>
		<line x1="20.3" y1="15.9" x2="23.3" y2="15.9" stroke="${lineColor}" stroke-width="0.5"/>
		<line x1="14.5" y1="15.9" x2="17.5" y2="15.9" stroke="${lineColor}" stroke-width="0.5"/>
		<line x1="8.7" y1="15.9" x2="11.7" y2="15.9" stroke="${lineColor}" stroke-width="0.5"/>
		<line x1="2.9" y1="15.9" x2="5.9" y2="15.9" stroke="${lineColor}" stroke-width="0.5"/>

		<!-- Row 3 -->
		<rect x="1.5" y="18.4" width="29" height="5" fill="${firstRowStripe}"/>
		<line x1="26.1" y1="20.8" x2="29.1" y2="20.8" stroke="${lineColor}" stroke-width="0.5"/>
		<line x1="20.3" y1="20.8" x2="23.3" y2="20.8" stroke="${lineColor}" stroke-width="0.5"/>
		<line x1="14.5" y1="20.8" x2="17.5" y2="20.8" stroke="${lineColor}" stroke-width="0.5"/>
		<line x1="8.7" y1="20.8" x2="11.7" y2="20.8" stroke="${lineColor}" stroke-width="0.5"/>
		<line x1="2.9" y1="20.8" x2="5.9" y2="20.8" stroke="${lineColor}" stroke-width="0.5"/>

		<!-- Row 4 -->
		<rect x="1.5" y="23.3" width="29" height="5" fill="${secondRowStripe}"/>
		<line x1="26.1" y1="25.8" x2="29.1" y2="25.8" stroke="${lineColor}" stroke-width="0.5"/>
		<line x1="20.3" y1="25.8" x2="23.3" y2="25.8" stroke="${lineColor}" stroke-width="0.5"/>
		<line x1="14.5" y1="25.8" x2="17.5" y2="25.8" stroke="${lineColor}" stroke-width="0.5"/>
		<line x1="8.7" y1="25.8" x2="11.7" y2="25.8" stroke="${lineColor}" stroke-width="0.5"/>
		<line x1="2.9" y1="25.8" x2="5.9" y2="25.8" stroke="${lineColor}" stroke-width="0.5"/>

	</svg>`;
}
