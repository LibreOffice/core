/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package com.sun.star.wizards.common;

/**
 * This Interface specifies a method of an object which is 
 * capable of reading adn writing its data out of the 
 * OO Configuration. <br/>
 * There are 2 direct implementations: ConfigGroup and ConfigSet.
 * The root is the first Java Object in the configuration hirarchie.
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
