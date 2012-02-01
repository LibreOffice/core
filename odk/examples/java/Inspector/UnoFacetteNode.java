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



import com.sun.star.reflection.XIdlMethod;
import com.sun.star.uno.Type;
import com.sun.star.uno.XComponentContext;
import javax.swing.tree.DefaultMutableTreeNode;

public class UnoFacetteNode extends UnoNode{
    String m_sFilter = "";


    /** Creates a new instance of UnoMethodNode */
    public UnoFacetteNode(Object _oUnoObject){
        super(_oUnoObject);
        addDummyNode();
    }


    public String getFilter(){
        return m_sFilter;
    }

    public void setFilter(String _sFilter){
        m_sFilter = _sFilter;
    }

    public String getName(){
        return toString();
    }

// TODO The implementation of the following nodes is not really robust and should be changed!!!
    public boolean isMethodNode(){
        return ((String) getUserObject()).equals(SMETHODDESCRIPTION);
    }

    public boolean isPropertyNode(){
        String sNodeDescription = (String) getUserObject();
        return ((sNodeDescription.equals(SPROPERTYDESCRIPTION)) || (sNodeDescription.equals(SPROPERTYINFODESCRIPTION)) || (sNodeDescription.equals(SPROPERTYVALUEDESCRIPTION)));
    }

    public boolean isContainerNode(){
        return ((String) getUserObject()).equals(SCONTAINERDESCRIPTION);
    }

    public boolean isServiceNode(){
        return ((String) getUserObject()).equals(SSERVICEDESCRIPTION);
    }

    public boolean isInterfaceNode(){
        return ((String) getUserObject()).equals(SINTERFACEDESCRIPTION);
    }
}

