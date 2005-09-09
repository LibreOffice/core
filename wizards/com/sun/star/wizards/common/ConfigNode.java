/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ConfigNode.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:18:39 $
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
 ************************************************************************/package com.sun.star.wizards.common;

/**
 * This Interface specifies a method of an object which is
 * capable of reading adn writing its data out of the
 * OO Configuration. <br/>
 * There are 2 direct implementations: ConfigGroup and ConfigSet.
 * The root is the first Java Object in the configuration hirarchie.
 * @author  rpiterman
 */
public interface ConfigNode {

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
