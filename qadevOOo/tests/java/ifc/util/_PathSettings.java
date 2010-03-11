/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package ifc.util;

import lib.MultiPropertyTest;
import util.utils;

import com.sun.star.lang.XMultiServiceFactory;

/**
 * Testing <code>com.sun.star.util.PathSettings</code>
 * service properties :
 * <ul>
 *  <li><code> Addin         </code></li>
 *  <li><code> AutoCorrect   </code></li>
 *  <li><code> AutoText      </code></li>
 *  <li><code> Backup        </code></li>
 *  <li><code> Basic         </code></li>
 *  <li><code> Bitmap        </code></li>
 *  <li><code> Config        </code></li>
 *  <li><code> Dictionary    </code></li>
 *  <li><code> Favorites     </code></li>
 *  <li><code> Filter        </code></li>
 *  <li><code> Gallery       </code></li>
 *  <li><code> Graphic       </code></li>
 *  <li><code> Help          </code></li>
 *  <li><code> Linguistic    </code></li>
 *  <li><code> Module        </code></li>
 *  <li><code> Palette       </code></li>
 *  <li><code> Plugin        </code></li>
 *  <li><code> Storage       </code></li>
 *  <li><code> Temp          </code></li>
 *  <li><code> Template      </code></li>
 *  <li><code> UIConfig      </code></li>
 *  <li><code> UserConfig    </code></li>
 *  <li><code> UserDictionary</code></li>
 *  <li><code> Work          </code></li>
 * </ul> <p>
 *
 * Properties testing is automated by <code>lib.MultiPropertyTest</code>.
 * @see com.sun.star.util.PathSettings
 */
public class _PathSettings extends MultiPropertyTest {

    /**
     * This path list could be empty after SO installation.
     * And the URL is validated to match the pattern 'file:///*'
     * thats why the custom changing of this property required.
     */
    public void _UIConfig() {
        testProperty("UIConfig", new PropertyTester() {
            public Object getNewValue(String propName, Object oldVal) {
                String path = (String) oldVal;
                if (path == null) path = "";
                if (path.length() > 0 && !path.endsWith(";")) {
                    path += ";";
                }
                path += utils.getOfficeTemp((XMultiServiceFactory)tParam.getMSF());
                return path;
            }
        });
    }
}

