package org.openoffice.netbeans.modules.office.options;

import java.awt.Image;
import java.beans.*;

import org.openide.ErrorManager;
import org.openide.util.NbBundle;
import org.openide.util.Utilities;

/** Description of {@link OfficeSettings}.
 *
 * @author tomaso
 */
public class OfficeSettingsBeanInfo extends SimpleBeanInfo {

    public PropertyDescriptor[] getPropertyDescriptors() {
        try {
            PropertyDescriptor[] props = new PropertyDescriptor[] {
                new PropertyDescriptor(OfficeSettings.OFFICE_DIRECTORY,
                               OfficeSettings.class,
                               "get" + OfficeSettings.OFFICE_DIRECTORY,
                               "set" + OfficeSettings.OFFICE_DIRECTORY),
                new PropertyDescriptor(OfficeSettings.WARN_BEFORE_DOC_DEPLOY,
                               OfficeSettings.class,
                               "get" + OfficeSettings.WARN_BEFORE_DOC_DEPLOY,
                               "set" + OfficeSettings.WARN_BEFORE_DOC_DEPLOY),
                new PropertyDescriptor(OfficeSettings.WARN_AFTER_DIR_DEPLOY,
                               OfficeSettings.class,
                               "get" + OfficeSettings.WARN_AFTER_DIR_DEPLOY,
                               "set" + OfficeSettings.WARN_AFTER_DIR_DEPLOY),
                new PropertyDescriptor(OfficeSettings.WARN_BEFORE_MOUNT,
                               OfficeSettings.class,
                               "get" + OfficeSettings.WARN_BEFORE_MOUNT,
                               "set" + OfficeSettings.WARN_BEFORE_MOUNT)
            };

            props[0].setDisplayName(NbBundle.getMessage(
                OfficeSettingsBeanInfo.class, "PROP_OfficeDirectory"));
            props[0].setShortDescription(NbBundle.getMessage(
                OfficeSettingsBeanInfo.class, "HINT_OfficeDirectory"));

            props[1].setDisplayName(NbBundle.getMessage(
                OfficeSettingsBeanInfo.class, "PROP_WarnBeforeDocDeploy"));
            props[1].setShortDescription(NbBundle.getMessage(
                OfficeSettingsBeanInfo.class, "HINT_WarnBeforeDocDeploy"));
            props[1].setHidden(true);

            props[2].setDisplayName(NbBundle.getMessage(
                OfficeSettingsBeanInfo.class, "PROP_WarnAfterDirDeploy"));
            props[2].setShortDescription(NbBundle.getMessage(
                OfficeSettingsBeanInfo.class, "HINT_WarnAfterDirDeploy"));
            props[2].setHidden(true);

            props[3].setDisplayName(NbBundle.getMessage(
                OfficeSettingsBeanInfo.class, "PROP_WarnBeforeMount"));
            props[3].setShortDescription(NbBundle.getMessage(
                OfficeSettingsBeanInfo.class, "HINT_WarnBeforeMount"));
            props[3].setHidden(true);

            return props;
        }
        catch (IntrospectionException ie) {
            ErrorManager.getDefault().notify(ie);
            return null;
        }
    }

    public Image getIcon(int type) {
        if (type == BeanInfo.ICON_COLOR_16x16 || type == BeanInfo.ICON_MONO_16x16) {
            return Utilities.loadImage("/org/openoffice/netbeans/modules/office/options/OfficeSettingsIcon.gif");
        } else {
            return Utilities.loadImage("/org/openoffice/netbeans/modules/office/options/OfficeSettingsIcon32.gif");
        }
    }
}
