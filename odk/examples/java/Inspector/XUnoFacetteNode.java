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





public interface XUnoFacetteNode extends XUnoNode {

    public static String SMETHODDESCRIPTION = "Methods";
    public static String SPROPERTYDESCRIPTION = "Properties";
    public static String SPROPERTYINFODESCRIPTION = "PropertyInfo";
    public static String SCONTAINERDESCRIPTION = "Container";
    public static String SSERVICEDESCRIPTION = "Services";
    public static String SINTERFACEDESCRIPTION = "Interfaces";
    public static String SCONTENTDESCRIPTION = "Content";
    public static String SPROPERTYVALUEDESCRIPTION = "PropertyValues";


    public boolean isMethodNode();

    public boolean isPropertyNode();

    public boolean isContainerNode();

    public boolean isServiceNode();

    public boolean isInterfaceNode();

    public String getFilter();

    public void setFilter(String _sFilter);

}
