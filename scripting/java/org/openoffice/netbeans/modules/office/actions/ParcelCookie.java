package org.openoffice.netbeans.modules.office.actions;

import java.io.File;
import org.openide.nodes.Node;

public interface ParcelCookie extends Node.Cookie
{
    public File getFile();

    public void mount();

    public boolean deploy(File target);
}
