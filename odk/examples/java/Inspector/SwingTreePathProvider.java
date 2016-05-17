/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

import javax.swing.tree.TreePath;

public class SwingTreePathProvider implements XTreePathProvider {
    private TreePath m_aTreePath;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
