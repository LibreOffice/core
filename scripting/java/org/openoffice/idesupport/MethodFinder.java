package org.openoffice.idesupport;

import java.io.File;
import com.sun.star.script.framework.container.ScriptEntry;

public interface MethodFinder {
    public ScriptEntry[] findMethods(File basedir);
}
