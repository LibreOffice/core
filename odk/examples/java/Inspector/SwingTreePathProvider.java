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

import javax.swing.tree.TreePath;


public class SwingTreePathProvider implements XTreePathProvider {
    TreePath m_aTreePath;

    /** Creates a new instance of TreePathProvider */
    public SwingTreePathProvider(TreePath _aTreePath) {
        m_aTreePath = _aTreePath;
    }


    public XUnoNode getLastPathComponent(){
        return (XUnoNode) m_aTreePath.getLastPathComponent();
    }


    public XUnoNode getPathComponent(int i){
        return (XUnoNode) m_aTreePath.getPathComponent(i);
    }

    public int getPathCount(){
        return m_aTreePath.getPathCount();
    }


    public XTreePathProvider getParentPath(){
        return new SwingTreePathProvider(m_aTreePath.getParentPath());
    }


    public XTreePathProvider pathByAddingChild(XUnoNode _oUnoNode){
        TreePath aTreePath = m_aTreePath.pathByAddingChild(_oUnoNode);
        return new SwingTreePathProvider(aTreePath);
    }

    public TreePath getSwingTreePath(){
        return m_aTreePath;
    }
}
