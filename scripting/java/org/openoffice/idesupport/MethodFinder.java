package org.openoffice.idesupport;

import java.io.File;

public interface MethodFinder {
    public ScriptEntry[] findMethods(File basedir);
}
