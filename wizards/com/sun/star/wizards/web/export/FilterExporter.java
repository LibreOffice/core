/*
 * Created on 17.10.2003
 *
 * To change the template for this generated file go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
package com.sun.star.wizards.web.export;

import com.sun.star.io.IOException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.Properties;
import com.sun.star.wizards.ui.event.Task;
import com.sun.star.wizards.web.data.CGDocument;
import com.sun.star.wizards.web.data.CGExporter;

/**
 * @author rpiterman
 *
 * An exporter which is configured with a filter name, and
 * uses the specified filter to export documents.
 */
public class FilterExporter extends AbstractExporter {

    protected String filterName;
    protected Properties props = new Properties();

    /* (non-Javadoc)
     * @see com.sun.star.wizards.web.export.Exporter#export(java.lang.Object, java.io.File, com.sun.star.wizards.web.data.CGSettings, com.sun.star.lang.XMultiServiceFactory)
     */
    public boolean export(
        CGDocument source,
        String target,
        XMultiServiceFactory xmsf,
        Task task
        ) {

        boolean result = true;
        Object document = null;

        try {
          document = openDocument(source,xmsf);
          task.advance(true);
          storeToURL(document,target, filterName, props.getProperties());
          task.advance(true);

        }
        catch (IOException iox) {
            result = false;
        }
        finally {
            closeDocument(document,xmsf);
            calcFileSize(source,target,xmsf);
            task.advance(true);
        }
        return true;
    }

    /* (non-Javadoc)
     * @see com.sun.star.wizards.web.export.Exporter#init(com.sun.star.wizards.web.data.CGExporter)
     */
    public void init(CGExporter exporter_) {
        super.init(exporter_);
        filterName = getArgument("Filter",exporter_);
    }

}
