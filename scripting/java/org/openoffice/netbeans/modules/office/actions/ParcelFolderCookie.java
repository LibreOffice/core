package org.openoffice.netbeans.modules.office.actions;

import org.openide.nodes.Node;

public interface ParcelFolderCookie extends Node.Cookie
{
    public void generate();

    public void configure();
}