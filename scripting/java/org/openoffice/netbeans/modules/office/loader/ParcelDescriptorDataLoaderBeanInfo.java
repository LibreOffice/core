package org.openoffice.netbeans.modules.office.loader;

import java.awt.Image;
import java.beans.*;

import org.openide.ErrorManager;
import org.openide.util.NbBundle;
import org.openide.util.Utilities;

/** Description of {@link ParcelDescriptorDataLoader}.
 *
 * @author tomaso
 */
public class ParcelDescriptorDataLoaderBeanInfo extends SimpleBeanInfo {

    // If you have additional properties:
    /*
    public PropertyDescriptor[] getPropertyDescriptors() {
        try {
            PropertyDescriptor myProp = new PropertyDescriptor("myProp", ParcelDescriptorDataLoader.class);
            myProp.setDisplayName(NbBundle.getMessage(ParcelDescriptorDataLoaderBeanInfo.class, "PROP_myProp"));
            myProp.setShortDescription(NbBundle.getMessage(ParcelDescriptorDataLoaderBeanInfo.class, "HINT_myProp"));
            return new PropertyDescriptor[] {myProp};
        } catch (IntrospectionException ie) {
            ErrorManager.getDefault().notify(ie);
            return null;
        }
    }
     */

    public BeanInfo[] getAdditionalBeanInfo() {
        try {
            // I.e. MultiFileLoader.class or UniFileLoader.class.
            return new BeanInfo[] {Introspector.getBeanInfo(ParcelDescriptorDataLoader.class.getSuperclass())};
        } catch (IntrospectionException ie) {
            ErrorManager.getDefault().notify(ie);
            return null;
        }
    }

    public Image getIcon(int type) {
        if (type == BeanInfo.ICON_COLOR_16x16 || type == BeanInfo.ICON_MONO_16x16) {
            return Utilities.loadImage("org/openoffice/netbeans/modules/office/loaders/ParcelDescriptorDataIcon.gif");
        } else {
            return Utilities.loadImage("org/openoffice/netbeans/modules/office/loaders/ParcelDescriptorDataIcon32.gif");
        }
    }

}
