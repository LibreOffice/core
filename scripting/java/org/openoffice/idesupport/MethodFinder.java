package org.openoffice.idesupport;

import java.io.File;
import com.sun.star.script.framework.browse.ScriptEntry;

public interface MethodFinder {
    public ScriptEntry[] findMethods(File basedir);
}
