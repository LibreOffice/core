/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ConverterCapabilities.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:18:15 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
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

package org.openoffice.xmerge;

/**
 *  <p>A <code>ConverterCapabilities</code> object is used by
 *  <code>DocumentMerger</code> implementations.  The
 *  <code>ConverterCapabilities</code> indicates which
 *  &quot;Office&quot; XML tags are supported by the
 *  &quot;Device&quot; format.</p>
 *
 *  @see  org.openoffice.xmerge.PluginFactory
 *  @see  org.openoffice.xmerge.DocumentMerger
 */
public interface ConverterCapabilities {


    /**
     *  Test to see if the device document format supports the
     *  tag in question.
     *
     *  @param  tag  The tag to check.
     *
     *  @return  true if the device format supports the
     *           tag, false otherwise.
     */
    boolean canConvertTag(String tag);


    /**
     *  Test to see if the device document format supports the
     *  tag attribute in question.
     *
     *  @param  tag        The tag to check.
     *  @param  attribute  The tag attribute to check.
     *
     *  @return  true if the device format supports the
     *          attribute, false otherwise.
     */
    boolean canConvertAttribute(String tag, String attribute);
}

