/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ConfigNode.java,v $
 * $Revision: 1.6 $
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
package com.sun.star.wizards.common;

/**
 * This Interface specifies a method of an object which is
 * capable of reading adn writing its data out of the
 * OO Configuration. <br/>
 * There are 2 direct implementations: ConfigGroup and ConfigSet.
 * The root is the first Java Object in the configuration hirarchie.
 * @author  rpiterman
 */
public interface ConfigNode
{

    /**
     * reads the object data out of the configuration.
     * @param  configurationView is a ::com::sun::star::configuration::HierarchyElement
     * which represents the node corresponding to the Object.
     * @param param a free parameter. Since the intension of this interface is
     * to be used in a tree like way, reading objects and subobjects and so on,
     * it might be practical to be able to pass an extra parameter, for a free use.
     */
    public void readConfiguration(Object configurationView, Object param);

    public void writeConfiguration(Object configurationView, Object param);

    public void setRoot(Object root);
}
