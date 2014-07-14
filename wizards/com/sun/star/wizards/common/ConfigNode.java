/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


package com.sun.star.wizards.common;

/**
 * This Interface specifies a method of an object which is
 * capable of reading and writing its data out of the
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
