<!-- ************************************************************************************************************************ -->
<!-- a name of a registered UNO service-->
<!ENTITY % service-name "CDATA">
<!-- chart2 *************************************************************************************************************** -->
<!ELEMENT chart2:chart2 (chart2:legend*, chart2:scale*, chart2:increment*, chart2:final-target+, chart2:resident*, table:table?)>
<!-- title ******************************************************************************************************************* -->
<!ELEMENT chart2:title EMPTY>
<!-- create your own name here to identifiy this title object for use in a ... ?-->
<!ATTLIST chart2:title
	id ID #REQUIRED
>
<!-- legend *************************************************************************************************************** -->
<!ELEMENT chart2:legend EMPTY>
<!-- create your own name here to identifiy this legend object for use in a plot-container or coordinate-system-->
<!ATTLIST chart2:legend
	id ID #REQUIRED
>
<!-- contains a name of a registered uno component implementing the service ...'legend' ... -->
<!ATTLIST chart2:legend
	service %service-name; #REQUIRED
>
<!-- final-target *********************************************************************************************************** -->
<!ELEMENT chart2:final-target (chart2:title?, chart2:logic-target)>
<!-- create your own name here to identifiy this final-target object -->
<!ATTLIST chart2:final-target
	id ID #REQUIRED
>
<!-- choose the id of a legend that should be used as default legend for all data series contained in this final target;
											the legend has to be described somewhere else in this document -->
<!ATTLIST chart2:final-target
	legend IDREF #IMPLIED
>
<!-- logic-target ********************************************************************************************************** -->
<!ELEMENT chart2:logic-target (chart2:coordinate-system+, chart2:logic-target*)>
<!-- create your own name here to identifiy this logic-target object -->
<!ATTLIST chart2:logic-target
	id ID #REQUIRED
>
<!-- choose the id of a legend that should be used as default legend for all data series contained in this logic target;
											the legend has to be described somewhere else in this document -->
<!ATTLIST chart2:logic-target
	legend IDREF #IMPLIED
>
<!-- coordinatesystem ************************************************************************************************** -->
<!ELEMENT chart2:coordinate-system (chart2:scaleRef*)>
<!-- create your own name here to identifiy this coordinatesystem object for use in a renderer-->
<!ATTLIST chart2:coordinate-system
	id ID #REQUIRED
>
<!-- contains a name of a registered uno component implementing the service ...'coordinate-system' ... -->
<!ATTLIST chart2:coordinate-system
	service %service-name; #REQUIRED
>
<!-- choose the id of a legend that should be used for this coordinate system;
											the legend has to be described somewhere else in this document -->
<!ATTLIST chart2:coordinate-system
	legend IDREF #IMPLIED
>
<!-- scale ***************************************************************************************************************** -->
<!ELEMENT chart2:scale (chart2:break*)>
<!-- create your own name here to identifiy this scale object for use in a coordinate system-->
<!ATTLIST chart2:scale
	id ID #REQUIRED
>
<!--  if the maximum value of the scale should be calculated automatically do not set this attribute-->
<!ATTLIST chart2:scale
	maximum %float; #IMPLIED
>
<!--  if the minimum value of the scale should be calculated automatically do not set this attribute-->
<!ATTLIST chart2:scale
	minimum %float; #IMPLIED
>
<!-- choose a name of a registered uno component implementing the service ...scaling' ... e.g. drafts.com.sun.star.LogarithmicScaling;
	if no scaling-service is set a linear scaling will be used-->
<!ATTLIST chart2:scale
	scaling-service %service-name; #IMPLIED
>
<!-- scaleRef ************************************************************************************************************* -->
<!ELEMENT chart2:scaleRef EMPTY>
<!-- choose the id of a scale that should be used here -->
<!ATTLIST chart2:scaleRef
	scale IDREF #REQUIRED
>
<!-- choose the direction in which the scale should be applied;
	if no direction is set the mathematical direction is used -->
<!-- !!!!!!!!!! .... maybe remove languagedependent here -->
<!ATTLIST chart2:scaleRef
	direction (mathematical | reverse | languagedependent) #IMPLIED
>
<!-- break ***************************************************************************************************************** -->
<!ELEMENT chart2:break EMPTY>
<!--  the value where the break starts and ends on the scale -->
<!ATTLIST chart2:break
	start %float; #REQUIRED
	end %float; #REQUIRED
>
<!-- increment ************************************************************************************************************ -->
<!ELEMENT chart2:increment (chart2:subincrement*)>
<!-- create your own name here to identifiy this increment object for use in an axis and/or grid -->
<!ATTLIST chart2:increment
	id ID #REQUIRED
>
<!-- choose the id of a scale that should be used for autocalculation of the values of this increment;
											the scale has to be described somewhere else in this document -->
<!ATTLIST chart2:increment
	scale IDREF #REQUIRED
>
<!-- an offset (measured in values before scaling) to which ticks are relatively positioned;
	if this attribute is not set 0 is assumed;
	if the given value is not valid in the associated scaling(s) the minimum of the scaling(s) is assumed )-->
<!ATTLIST chart2:increment
	pre-offset %float; #IMPLIED
>
<!-- a distance (measured in values before scaling) how far ticks are seperated one from each other;
	this value is ignored as long as the attribute post-equidistant is true;
	if attribute post-equidistant is false and this value is missing it will be automatically calculated -->
<!ATTLIST chart2:increment
	pre-distance %float; #IMPLIED
>
<!-- a distance (measured in values after scaling) how far ticks are seperated one from each other
	this value is ignored as long as the attribute post-equidistant is false;
	if attribute post-equidistant is true and this value is missing it will be automatically calculated -->
<!ATTLIST chart2:increment
	post-distance %float; #IMPLIED
>
<!-- if the post-equidistant attribute is missing or set to false tickmarks will be positioned equidistant measured in values before scaling otherwise equidistant after scaling;
	for example lets assume to have a logarithmic scaling - then with post-equidistant=true and post-distance=1 you will get tichmarks at 0, 1, 10, 100 etc.
	having post-equidistant=false and pre-distance=1 you will have tickmarks at 0,1,2,3,4 etc.-->
<!ATTLIST chart2:increment
	post-equidistant %boolean; #IMPLIED
>
<!-- subincrement ************************************************************************************************************ -->
<!ELEMENT chart2:subincrement EMPTY>
<!-- any parent range will be divided into this count of sub-ranges; values lower than 2 have no effect;
	if no value is set the count will be calculated automatically -->
<!ATTLIST chart2:subincrement
	count %integer; #IMPLIED
>
<!-- if the post-equidistant attribute is missing or set to false the sub-ranges will be equal to each other measured in values before scaling otherwise measured after scaling -->
<!ATTLIST chart2:subincrement
	post-equidistant %boolean; #IMPLIED
>
<!-- resident ************************************************************************************************************** -->
<!ELEMENT chart2:resident ANY>
<!-- contained child elements could be for example (chart2:axis|chart2:grid|chart2:renderer) -->
<!-- create your own name to identifiy this resident object -->
<!ATTLIST chart2:resident
	id ID #REQUIRED
>
<!-- contains a name of a registered UNO component supporting the service ... or interface?...'resident' ...;
if a child element is contained there are additional requirements to the UNO component:
axis: the service ... 'axis' needs to be supported
grid: the service ... 'grid' needs to be supported
renderer:  the service ... 'datarenderer' needs to be supported
-->
<!ATTLIST chart2:resident
	service %service-name; #REQUIRED
>
<!-- choose the id of a coordinatesystem that should be used;
											that coordinatesystem has to be described somewhere else in this document -->
<!ATTLIST chart2:resident
	coordinate-system IDREF #REQUIRED
>
<!-- axis ******************************************************************************************************************* -->
<!ELEMENT chart2:axis EMPTY>
<!-- state which dimension of the coordinate system the axis should use (e.g. for cartesian coordinates x is dimension 1 and y is dimansion 2)-->
<!ATTLIST chart2:axis
	dimension %integer; #REQUIRED
>
<!-- choose the id of an increment that should be used;
											that increment has to be described somewhere else in this document -->
<!ATTLIST chart2:axis
	increment IDREF #REQUIRED
>
<!-- grid ******************************************************************************************************************* -->
<!ELEMENT chart2:grid EMPTY>
<!-- state which dimension of the coordinate system the axis should use (e.g. for cartesian coordinates x is dimension 1 and y is dimansion 2)-->
<!ATTLIST chart2:grid
	dimension %integer; #REQUIRED
>
<!-- choose the id of an increment that should be used;
											that increment has to be described somewhere else in this document -->
<!ATTLIST chart2:grid
	increment IDREF #REQUIRED
>
<!-- renderer ************************************************************************************************************* -->
<!ELEMENT chart2:renderer EMPTY>