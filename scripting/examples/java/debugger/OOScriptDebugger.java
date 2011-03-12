import java.io.InputStream;
import drafts.com.sun.star.script.framework.runtime.XScriptContext;

public interface OOScriptDebugger {
    public void go(XScriptContext ctxt, String filename);
    public void go(XScriptContext ctxt, InputStream in);
}
