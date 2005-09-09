<!--

    OpenOffice.org - a multi-platform office productivity suite
 
    $RCSfile: defs.mod,v $
 
    $Revision: 1.3 $
 
    last change: $Author: rt $ $Date: 2005-09-09 12:29:57 $
 
    The Contents of this file are made available subject to
    the terms of GNU Lesser General Public License Version 2.1.
 
 
      GNU Lesser General Public License Version 2.1
      =============================================
      Copyright 2005 by Sun Microsystems, Inc.
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
 
-->

<!-- This module should contain entities intended for content definitions
     in several other modules. Putting all of them here should remove
     (some) order dependencies of the other module files
-->


<!-- text marks for tracking changes; usually used inside of paragraphs -->
<!ENTITY % change-marks "text:change | text:change-start | text:change-end">

<!-- (optional) text declarations; used before the first paragraph -->
<!ENTITY % text-decls "text:variable-decls?, text:sequence-decls?,
					   text:user-field-decls?, text:dde-connection-decls?, 
					   text:alphabetical-index-auto-mark-file?" >

<!-- define the types of text which may occur inside of sections -->
<!ENTITY % sectionText "(text:h|text:p|text:ordered-list|
						text:unordered-list|table:table|text:section|
						text:table-of-content|text:illustration-index|
						text:table-index|text:object-index|text:user-index|
						text:alphabetical-index|text:bibliography|
						text:index-title|%change-marks;)*">

<!ENTITY % headerText "(%text-decls;, (text:h|text:p|text:ordered-list|
						text:unordered-list|table:table|text:section|
						text:table-of-content|text:illustration-index|
						text:table-index|text:object-index|text:user-index|
						text:alphabetical-index|text:bibliography|
						text:index-title|%change-marks;)* )">

