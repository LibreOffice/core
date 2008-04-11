/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: IMapping.java,v $
 * $Revision: 1.5 $
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

package com.sun.star.uno;

/**
 * With a mapping objets can be mapped from one environment to another.
 *
 * <p>This interface exists for compatibility with the binary UNO API.</p>
 *
 * @see com.sun.star.uno.IBridge
 *
 * @deprecated As of UDK 3.2, this interface is deprecated, without offering a
 * replacement.
 */
public interface IMapping {
    /**
     * Maps an interface from one environment to another.
     *
     * @param object source object that is to be mapped
     * @param type description of the interface that is to be mapped
     * @return the object mapped to the destination environment
     */
    Object mapInterface(Object object, Type type);
}
