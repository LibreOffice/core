/************************************************************************
 *
 *  BeforeAfter.java
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
 *  Copyright: 2002-2004 by Henrik Just
 *
 *  All Rights Reserved.
 *
 *  Version 0.3.3f (2004-08-26)
 *
 */

package writer2latex.latex.style;

/* Utility class to hold LaTeX code to put before/after other LaTeX code
 */
public class BeforeAfter {
    private String sBefore="";
    private String sAfter="";

    public void add(String sBefore1, String sAfter1) {
        sBefore+=sBefore1; sAfter=sAfter1+sAfter;
    }

    public String getBefore() { return sBefore; }
    public String getAfter() { return sAfter; }

    public boolean isEmpty() { return sBefore.length()==0 && sAfter.length()==0; }

}