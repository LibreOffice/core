<!--
	$Id: form.mod,v 1.5 2001-02-26 14:53:39 cl Exp $

   The Contents of this file are made available subject to the terms of
   either of the following licenses

          - GNU Lesser General Public License Version 2.1
          - Sun Industry Standards Source License Version 1.1

   Sun Microsystems Inc., October, 2000

   GNU Lesser General Public License Version 2.1
   =============================================
   Copyright 2000 by Sun Microsystems, Inc.
   901 San Antonio Road, Palo Alto, CA 94303, USA

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License version 2.1, as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
   MA  02111-1307  USA


   Sun Industry Standards Source License Version 1.1
   =================================================
   The contents of this file are subject to the Sun Industry Standards
   Source License Version 1.1 (the "License"); You may not use this file
   except in compliance with the License. You may obtain a copy of the
   License at http://www.openoffice.org/license.html.

   Software provided under this License is provided on an "AS IS" basis,
   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
   See the License for the specific provisions governing your rights and
   obligations concerning the Software.

   The Initial Developer of the Original Code is: Sun Microsystems, Inc.

   Copyright: 2000 by Sun Microsystems, Inc.

   All Rights Reserved.

   Contributor(s): _______________________________________

-->

<!ENTITY % controls	"(	form:text|form:textarea|form:fixed-text|form:file|
						form:password|form:formatted-text|form:button|form:image|
						form:checkbox|form:radio|form:listbox|form:combobox|form:frame|
						form:hidden|form:image-frame|form:grid|form:generic-control)">

<!ENTITY % name "form:name CDATA #IMPLIED">
<!ENTITY % service-name "form:service-name CDATA #IMPLIED">

<!ENTITY % navigation "(none|current|parent)">
<!ENTITY % cycles "(records|current|page)">
<!ENTITY % url "CDATA">

<!ELEMENT form:control (%controls;+)>
<!ATTLIST form:control %name;
                       %service-name;
                       %control-id;>

<!ELEMENT form:form (form:properties?, office:events?, (form:control|form:form)*)>
<!ATTLIST form:form %name; %service-name;>
<!ATTLIST form:form xlink:href %url; #IMPLIED>
<!ATTLIST form:form form:enctype CDATA "application/x-www-form-urlencoded">
<!ATTLIST form:form form:method CDATA "get">
<!ATTLIST form:form office:target-frame CDATA "_blank">
<!ATTLIST form:form form:allow-deletes %boolean; "true">
<!ATTLIST form:form form:allow-inserts %boolean; "true">
<!ATTLIST form:form form:allow-updates %boolean; "true">
<!ATTLIST form:form form:apply-filter %boolean; "false">
<!ATTLIST form:form form:command CDATA #IMPLIED>
<!ATTLIST form:form form:command-type (table|query|command) "command">
<!ATTLIST form:form form:datasource CDATA #IMPLIED>
<!ATTLIST form:form form:detail-fields CDATA #IMPLIED>
<!ATTLIST form:form form:escape-processing %boolean; "true">
<!ATTLIST form:form form:filter CDATA #IMPLIED>
<!ATTLIST form:form form:ignore-result %boolean; "false">
<!ATTLIST form:form form:master-fields CDATA #IMPLIED>
<!ATTLIST form:form form:navigation-mode %navigation; #IMPLIED>
<!ATTLIST form:form form:order CDATA #IMPLIED>
<!ATTLIST form:form form:tab-cycle %cycles; "records" >

<!ELEMENT office:forms (form:form*)>

<!ELEMENT form:text (form:properties?, office:events?)>
<!ATTLIST form:text %current-value;
                    %disabled;
                    %max-length;
                    %printable;
                    %readonly;
                    %tab-index;
                    %tab-stop;
                    %title;
                    %value;
                    %convert-empty;
                    %data-field;>

<!ELEMENT form:textarea (form:properties?, script:events?)>
<!ATTLIST form:textarea %current-value; >
<!ELEMENT form:textarea (form:properties?, office:events?)>
<!ATTLIST form:textarea %current-value;
                        %disabled;
                        %max-length;
                        %printable;
                        %readonly;
                        %tab-index;
                        %tab-stop;
                        %title;
                        %value;
                        %convert-empty;
                        %data-field;>

<!ELEMENT form:password (form:properties?, office:events?)>
<!ATTLIST form:password %disabled;
                        %max-length;
                        %printable;
                        %tab-index;
                        %tab-stop;
                        %title;
                        %value;
						%convert-empty;>

<!ATTLIST form:password form:echo-char CDATA "*">

<!ELEMENT form:file (form:properties?, office:events?)>
<!ATTLIST form:file %current-value;
                    %disabled;
                    %max-length;
                    %printable;
                    %readonly;
                    %tab-index;
                    %tab-stop;
                    %title;
                    %value;>

<!ELEMENT form:formatted-text (form:properties?, office:events?)>
<!ATTLIST form:formatted-text %current-value;
                              %disabled;
                              %max-length;
                              %printable;
                              %readonly;
                              %tab-index;
                              %tab-stop;
                              %title;
                              %value;
                              %convert-empty;
                              %data-field;>
<!ATTLIST form:formatted-text form:max-value CDATA #IMPLIED>
<!ATTLIST form:formatted-text form:min-value CDATA #IMPLIED>
<!ATTLIST form:formatted-text form:validation %boolean; "false">

<!ELEMENT form:fixed-text (form:properties?, office:events?)>
<!ATTLIST form:fixed-text %for;
                          %disabled;
                          %label;
                          %printable;
                          %title;>
<!ATTLIST form:fixed-text form:multi-line %boolean; "false">

<!ELEMENT form:combobox (form:properties?, office:events?, form:item*)>
<!ATTLIST form:combobox %current-value;
                        %disabled;
                        %dropdown;
                        %max-length;
                        %printable;
                        %readonly;
                        %size;
                        %tab-index;
                        %tab-stop;
                        %title;
                        %value;
                        %convert-empty;
                        %data-field;
                        %list-source;
                        %list-source-type;>
<!ATTLIST form:combobox form:auto-complete %boolean; #IMPLIED>

<!ELEMENT form:item (#PCDATA)>
<!ATTLIST form:item %label;>

<!ELEMENT form:listbox (form:properties?, office:events?, form:option*)>
<!ATTLIST form:listbox %disabled;
                       %dropdown;
                       %printable;
                       %size;
                       %tab-index;
                       %tab-stop;
                       %title;
                       %bound-column;
                       %data-field;
                       %list-source;
                       %list-source-type;>
<!ATTLIST form:listbox form:multiple %boolean; "false">

<!ELEMENT form:option (#PCDATA)>
<!ATTLIST form:option %current-selected;
                      %selected;
                      %label;
                      %value;>

<!ELEMENT form:button (form:properties?, office:events?)>
<!ATTLIST form:button %button-type;
                      %disabled;
                      %label;
                      %image-data;
                      %printable;
                      %tab-index;
                      %tab-stop;
                      %target-frame;
                      %target-location;
                      %title;
                      %value;>
<!ATTLIST form:button form:default-button %boolean; "false">

<!ELEMENT form:image (form:properties?, office:events?)>
<!ATTLIST form:image %button-type;
                     %disabled;
                     %image-data;
                     %printable;
                     %tab-index;
                     %tab-stop;
                     %target-frame;
                     %target-location;
                     %title;
                     %value;>

<!ELEMENT form:checkbox (form:properties?, office:events?)>
<!ATTLIST form:checkbox %disabled;
                        %label;
                        %printable;
                        %tab-index;
                        %tab-stop;
                        %title;
                        %value;
                        %data-field;>
<!ENTITY % states "(unchecked|checked|unknown)">
<!ATTLIST form:checkbox form:current-state %states; #IMPLIED>
<!ATTLIST form:checkbox form:is-tristate %boolean; "false">
<!ATTLIST form:checkbox form:state %states; "unchecked">

<!ELEMENT form:radio (form:properties?, office:events?)>
<!ATTLIST form:radio %current-selected;
                     %disabled;
                     %label;
                     %printable;
                     %selected;
                     %tab-index;
                     %tab-stop;
                     %title;
                     %value;
                     %data-field;>

<!ELEMENT form:frame (form:properties?, office:events?)>
<!ATTLIST form:frame %disabled;
                     %for;
                     %label;
                     %printable;
                     %title;>

<!ELEMENT form:image-frame (form:properties?, office:events?)>
<!ATTLIST form:image-frame %disabled;
                           %image-data;
                           %printable;
                           %readonly;
                           %title;
                           %data-field;>

<!ELEMENT form:hidden (form:properties?, office:events?)>
<!ATTLIST form:hidden %name;
                      %service-name;
                      %value;>

<!ELEMENT form:grid (form:properties?, office:events?, form:column*)>
<!ATTLIST form:grid %disabled;
                    %printable;
                    %tab-index;
                    %tab-stop;
                    %title;>
<!ENTITY % column-type "(form:text| form:textarea| form:formatted-text|form:checkbox| form:listbox| form:combobox)">
<!ELEMENT form:column (%column-type;+)>
<!ATTLIST form:column %name;
                      %service-name;
                      %label;>

<!ELEMENT form:generic-control (form:properties?, office:events?)>



<!ENTITY % types "(submit|reset|push|url)">
<!ENTITY % button-type "form:button-type %types; 'push'">
<!ENTITY % control-id "form:id CDATA #REQUIRED">
<!ENTITY % current-selected "form:current-selected %boolean; 'false'">
<!ENTITY % current-value "form:current-value CDATA #IMPLIED">
<!ENTITY % value "form:value CDATA #IMPLIED">
<!ENTITY % disabled "form:disabled %boolean; 'false'">
<!ENTITY % dropdown "form:dropdown %boolean; 'false'">
<!ENTITY % for "form:for CDATA #IMPLIED">
<!ENTITY % image-data "form:image-data %url; #IMPLIED">
<!ENTITY % label "form:label CDATA #IMPLIED">
<!ENTITY % max-length "form:max-length CDATA #IMPLIED">
<!ENTITY % printable "form:printable %boolean; 'true'">
<!ENTITY % readonly "form:readonly %boolean; 'false'">
<!ENTITY % size "form:size CDATA #IMPLIED">
<!ENTITY % selected "form:selected %boolean; 'false'">
<!ENTITY % size "form:size CDATA #IMPLIED">
<!ENTITY % tab-index "form:tab-index CDATA #IMPLIED">
<!ENTITY % target-frame "office:target-frame CDATA '_blank'">
<!ENTITY % target-location "xlink:href %url; #IMPLIED">
<!ENTITY % tab-stop "form:tab-stop %boolean; 'true'">
<!ENTITY % title "form:title CDATA #IMPLIED">
<!ENTITY % default-value "form:default-value CDATA #IMPLIED">
<!ENTITY % bound-column "form:bound-column CDATA #IMPLIED">
<!ENTITY % convert-empty "form:convert-empty-to-null  %boolean; 'false'">
<!ENTITY % data-field "form:data-field CDATA #IMPLIED">
<!ENTITY % list-source "form:list-source CDATA #IMPLIED">
<!ENTITY % list-source-types "(table|query|sql|sql-pass-through|value-list|table-fields)">
<!ENTITY % list-source-type "form:list-source-type %list-source-types; #IMPLIED">


<!ELEMENT form:properties (form:property+)>
<!ELEMENT form:property (form:property-value*)>
<!ATTLIST form:property form:property-is-list %boolean; #IMPLIED>
<!ATTLIST form:property form:property-name CDATA #REQUIRED>
<!ATTLIST form:property form:property-type (boolean|short|int|long|double|string)  #REQUIRED>
<!ELEMENT form:property-value (#PCDATA)>

