/*
 * Created on 17.10.2003
 *
 * To change the template for this generated file go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
package com.sun.star.wizards.web.export;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.FileAccess;
import com.sun.star.wizards.ui.event.Task;
import com.sun.star.wizards.web.data.CGDocument;

/**
 * @author rpiterman
 *
 * To change the template for this generated type comment go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
public class CopyExporter extends AbstractExporter{


    /* (non-Javadoc)
     * @see com.sun.star.wizards.web.export.Exporter#export(java.lang.Object, java.io.File, com.sun.star.wizards.web.data.CGSettings, com.sun.star.lang.XMultiServiceFactory)
     */
    public boolean export(CGDocument source, String  target,XMultiServiceFactory xmsf, Task task) {
        try {
            task.advance(true);

            if (exporter.cp_PageType > 0) {
                closeDocument(openDocument(source,xmsf),xmsf);
            }

            task.advance(true);

            String newTarget = FileAccess.connectURLs(
                FileAccess.getParentDir(target) , source.urlFilename) ;

            boolean b = getFileAccess(xmsf).copy(source.cp_URL, newTarget);

            task.advance(true);

            calcFileSize(source,newTarget,xmsf);

            return b;

        }
        catch (Exception ex) {
            return false;
        }
    }


}
