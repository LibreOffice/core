/*
 * SetItem.java
 *
 * Created on 16. September 2003, 12:29
 */

package com.sun.star.wizards.web.data;

import com.sun.star.wizards.common.*;

/**
 *
 * @author  rpiterman
 */
public class ConfigSetItem extends ConfigGroup implements Indexable {
    public int cp_Index = -1;

    public int getIndex() {
        return cp_Index;
    }

    public CGSettings getSettings() {
        return (CGSettings)root;
    }
}
