/************************************************************************
 *
 *  L10n.java
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *  Copyright: 2002 by Henrik Just
 *
 *  All Rights Reserved.
 *
 *  Version 0.3.3g (2004-10-22)
 *
 */

package writer2latex.util;

// This class handles localized strings
public class L10n {
    public final static int FIRST = 0;
    public final static int PREVIOUS = 1;
    public final static int NEXT = 2;
    public final static int LAST = 3;
    public final static int CONTENTS = 4;
    public final static int INDEX = 5;

    private String sLocale="en-US";

    public void setLocale(String sLocale) {
        if (sLocale!=null) { this.sLocale = sLocale;}
    }

    public String get(int nString) {
        if (sLocale.startsWith("da")) { // danish
            switch (nString) {
                case FIRST : return "F\u00F8rste";
                case PREVIOUS : return "Forrige";
                case NEXT : return "N\u00E6ste";
                case LAST : return "Sidste";
                case CONTENTS : return "Indhold";
                case INDEX : return "Stikord";
            }
        }
        if (sLocale.startsWith("hr")) { // croatian
            switch (nString) {
                case FIRST : return "Prvi";
                case PREVIOUS : return "Prethodan";
                case NEXT : return "slijede\u0107i";
                case LAST : return "Zadnji";
                case CONTENTS : return "Sadr\u017Eaj";
                case INDEX : return "Indeks";
            }
        }
        else { // english - default
            switch (nString) {
                case FIRST : return "First";
                case PREVIOUS : return "Previous";
                case NEXT : return "Next";
                case LAST: return "Last";
                case CONTENTS : return "Contents";
                case INDEX : return "Index";
            }
        }
        return "???";
    }
}
