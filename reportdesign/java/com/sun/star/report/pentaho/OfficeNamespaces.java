/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OfficeNamespaces.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:28:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *    Copyright 2007 by Pentaho Corporation
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
package com.sun.star.report.pentaho;

/**
 * Creation-Date: Feb 22, 2007, 1:53:29 PM
 *
 * @author Thomas Morgner
 */
public class OfficeNamespaces
{

    public static final String OFFICE_NS = "urn:oasis:names:tc:opendocument:xmlns:office:1.0";
    public static final String TABLE_NS = "urn:oasis:names:tc:opendocument:xmlns:table:1.0";
    public static final String TEXT_NS = "urn:oasis:names:tc:opendocument:xmlns:text:1.0";
    public static final String STYLE_NS = "urn:oasis:names:tc:opendocument:xmlns:style:1.0";
    public static final String DRAWING_NS = "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0";
    public static final String FO_NS = "urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0";
    public static final String XLINK_NS = "http://www.w3.org/1999/xlink";
    public static final String PURL_NS = "http://purl.org/dc/elements/1.1/";
    public static final String META_NS = "urn:oasis:names:tc:opendocument:xmlns:meta:1.0";
    public static final String DATASTYLE_NS = "urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0";
    public static final String SVG_NS = "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0";
    public static final String CHART_NS = "urn:oasis:names:tc:opendocument:xmlns:chart:1.0";
    public static final String DR3D_NS = "urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0";
    public static final String MATHML_NS = "http://www.w3.org/1998/Math/MathML";
    public static final String FORM_NS = "urn:oasis:names:tc:opendocument:xmlns:form:1.0";
    public static final String SCRIPT_NS = "urn:oasis:names:tc:opendocument:xmlns:script:1.0";
    public static final String OO2004_NS = "http://openoffice.org/2004/office";
    public static final String OOW2004_NS = "http://openoffice.org/2004/writer";
    public static final String OOC2004_NS = "http://openoffice.org/2004/calc";
    public static final String XML_EVENT_NS = "http://www.w3.org/2001/xml-events";
    public static final String XFORMS_NS = "http://www.w3.org/2002/xforms";
    public static final String XSD_NS = "http://www.w3.org/2001/XMLSchema";
    public static final String XSI_NS = "http://www.w3.org/2001/XMLSchema-instance";
    public static final String OOREPORT_NS = "http://openoffice.org/2005/report";
    public static final String CONFIG = "urn:oasis:names:tc:opendocument:xmlns:config:1.0";
    public static final String INTERNAL_NS = "http://reporting.pentaho.org/namespaces/engine";
    /**
     * @deprecated
     */
    public static final String NUMBER_NS = DATASTYLE_NS;

    private OfficeNamespaces()
    {
    }
}
