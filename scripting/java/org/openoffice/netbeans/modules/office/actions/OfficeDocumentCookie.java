package org.openoffice.netbeans.modules.office.actions;

import java.util.Enumeration;
import javax.swing.event.ChangeListener;
import org.openide.nodes.Node;

public interface OfficeDocumentCookie extends Node.Cookie
{
    public void mount();
    public Enumeration getParcels();
    public void removeParcel(String name);

    public void addChangeListener(ChangeListener cl);
    public void removeChangeListener(ChangeListener cl);
}