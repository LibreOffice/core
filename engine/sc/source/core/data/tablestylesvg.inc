// Table-style swatch SVG templates, ported verbatim from the former
// browser Util.GenerateTableIcon.ts. Placeholders ${...} are substituted with
// the style's resolved colors before rasterizing. Do not edit by hand.

namespace
{
constexpr std::string_view gaTableStyleSvgTemplates[] = {
    // Light 1-7
    std::string_view(R"tsvg(<svg viewBox="0 0 32 32" fill="none" xmlns="http://www.w3.org/2000/svg">

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

		</svg>)tsvg"),
    // Light 8-14
    std::string_view(R"tsvg(<svg width="32" height="32" viewBox="0 0 32 32" fill="none" xmlns="http://www.w3.org/2000/svg">
			<!-- Background -->
			<rect id="table-background" x="1.7" y="3.2" width="28.6" height="25.6" fill="#FFFFFF"/>

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
		</svg>)tsvg"),
    // Light 15-21
    std::string_view(R"tsvg(<svg viewBox="0 0 32 32" fill="none" xmlns="http://www.w3.org/2000/svg">
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
	</svg>)tsvg"),
    // Medium 1-7
    std::string_view(R"tsvg(<svg viewBox="0 0 32 32" fill="none" xmlns="http://www.w3.org/2000/svg">
  <!-- Background -->
  <rect id="table-background" x="1.7" y="3.2" width="28.6" height="25.6" fill="#FFFFFF"/>

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
</svg>)tsvg"),
    // Medium 8-15
    std::string_view(R"tsvg(<svg viewBox="0 0 32 32" fill="none" xmlns="http://www.w3.org/2000/svg">
  <!-- Background -->
  <rect id="table-background" x="1.7" y="3.2" width="28.6" height="25.6" fill="#FFFFFF"/>

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
</svg>)tsvg"),
    // Medium 16-21
    std::string_view(R"tsvg(<svg viewBox="0 0 32 32" fill="none" xmlns="http://www.w3.org/2000/svg">

  <!-- Background -->
  <rect id="table-background" x="1.5" y="3.35" width="29" height="25.1" fill="#FFFFFF"/>

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

</svg>)tsvg"),
    // Medium 22-28
    std::string_view(R"tsvg(<svg viewBox="0 0 32 32" fill="none" xmlns="http://www.w3.org/2000/svg">
  <!-- Background -->
  <rect id="table-background" x="1.7" y="3.2" width="28.6" height="25.6" fill="#FFFFFF"/>

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
</svg>)tsvg"),
    // Dark
    std::string_view(R"tsvg(<svg viewBox="0 0 32 32" xmlns="http://www.w3.org/2000/svg">

		<!-- Background -->
		<rect id="table-background" x="1.5" y="2.95" width="29" height="25.35" fill="#FFFFFF"/>

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

	</svg>)tsvg"),
    // Custom
    std::string_view(R"tsvg(<svg viewBox="0 0 32 32" fill="none" xmlns="http://www.w3.org/2000/svg">

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

	</svg>)tsvg"),
};
}
