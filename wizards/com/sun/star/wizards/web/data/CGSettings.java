/*
 * IconSet.java
 *
 * Created on 12. September 2003, 17:16
 */

package com.sun.star.wizards.web.data;


import java.util.Hashtable;
import java.util.Map;
import java.util.Vector;

import com.sun.star.beans.PropertyValue;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.*;

/**
 * @author  rpiterman
 */


public class CGSettings    extends ConfigGroup {

    public String          soTemplateDir;
    public String          soGalleryDir;
    public String          workPath = null;

    public String          cp_WorkDir;

    public ConfigSet       cp_Exporters         = new ConfigSet(CGExporter.class);
    public ConfigSet       cp_Layouts           = new ConfigSet(CGLayout.class);
    public ConfigSet       cp_Styles            = new ConfigSet(CGStyle.class);
    public ConfigSet       cp_IconSets          = new ConfigSet(CGIconSet.class);
    public ConfigSet       cp_BackgroundImages  = new ConfigSet(CGImage.class);
    public ConfigSet       cp_SavedSessions     = new ConfigSet(CGSessionName.class);

    public CGSession       cp_DefaultSession    = new CGSession();

    private Map exportersMap = new Hashtable();


    public String resPages, resSlides, resCreated, resUpdated;

    public CGSettings(XMultiServiceFactory xmsf) {
        try {
            soTemplateDir = FileAccess.getOfficePath(xmsf, "Template","share");
            soGalleryDir  = FileAccess.getOfficePath(xmsf,"Gallery","share");
            root = this;
        }
        catch (NoValidPathException ex) {
          ex.printStackTrace();
        }
    }


    private static final CGExporter[] EMPTY_ARRAY_1 = new CGExporter[0];

    public CGExporter[] getExporters(String mime) {
        CGExporter[] exps = (CGExporter[])exportersMap.get(mime);
        if (exps == null)
          exportersMap.put(mime,exps = createExporters(mime));
        return exps;
    }

    private CGExporter[] createExporters(String mime) {
        Object[] exporters = cp_Exporters.items();
        Vector v = new Vector();
        for (int i = 0; i<exporters.length; i++)
          if (((CGExporter)exporters[i]).supports(mime)) {
                  try {
                    v.add(exporters[i]);
                  }
                  catch (Exception ex) {
                      ex.printStackTrace();
                  }
          }
        return (CGExporter[])v.toArray(EMPTY_ARRAY_1);
    }

    /**
     * call after read.
     * @param xmsf
     */
    public void configure(XMultiServiceFactory xmsf) throws Exception {
        workPath = FileAccess.connectURLs(soTemplateDir , cp_WorkDir);
        calcExportersTargetTypeNames(xmsf);
    }

    private void calcExportersTargetTypeNames(XMultiServiceFactory xmsf) throws Exception {
        Object typeDetect = xmsf.createInstance("com.sun.star.document.TypeDetection");
        XNameAccess xNameAccess = (XNameAccess)UnoRuntime.queryInterface(XNameAccess.class,typeDetect);
        for (int i = 0; i < cp_Exporters.getSize(); i++)
            calcExporterTargetTypeName(xNameAccess,(CGExporter)cp_Exporters.getElementAt(i));
    }


    private void calcExporterTargetTypeName(XNameAccess xNameAccess, CGExporter exporter)
        throws  NoSuchElementException,
                WrappedTargetException
    {
        if (!exporter.cp_TargetType.equals(""))
            exporter.targetTypeName =
                (String) Properties.getPropertyValue(
                    (PropertyValue[]) xNameAccess.getByName(exporter.cp_TargetType),
                    "UIName");

    }

}
