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

import com.sun.star.uno.Type;



public interface XUnoNode {

    public static final int nINTERFACE = 1;
    public static final int nSERVICE = 2;
    public static final int nOTHERS = 3;

    public String getAnchor();

    public String getClassName();

    public void openIdlDescription(String _sIDLUrl);

    public boolean isFilterApplicable(String _sFilter);

    public void setVisible(String _sFilter);

    public void setParameterObjects(Object[] _oParamObjects);

    public Object[] getParameterObjects();

    public String getName();

    public void setFoldable(boolean _bIsFoldable);

    public Object getUnoObject();

    public XUnoNode getParentNode();

    public void addChildNode(XUnoNode _xUnoNode);

    public void  setLabel(String _sLabel);

    public String getLabel();

    public int getChildCount();

    public XUnoNode getChild(int _index);

    public int getNodeType();

    // possible values are nINTERFACE, nSERVICE, nOTHERS
    public void setNodeType(int _nNodeType);

    /** delivers only the Uno-Type when the NodeType is set to nINTERFACE
     *  otherwise (when the NodeType has not been set returns null
     */
    public Type getUnoType();
}
