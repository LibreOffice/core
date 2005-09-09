/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _PathSettings.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:39:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

