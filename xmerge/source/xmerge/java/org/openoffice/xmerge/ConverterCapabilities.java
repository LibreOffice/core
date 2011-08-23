/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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

