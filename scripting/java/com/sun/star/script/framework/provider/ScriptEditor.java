package com.sun.star.script.framework.provider;

import drafts.com.sun.star.script.provider.XScriptContext;
import com.sun.star.script.framework.container.ScriptMetaData;

public interface ScriptEditor
{
    public void edit(XScriptContext context, ScriptMetaData entry);
    public String getTemplate();
    public String getExtension();
}
