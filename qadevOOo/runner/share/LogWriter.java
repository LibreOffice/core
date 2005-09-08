/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: LogWriter.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:26:48 $
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

package share;

public interface LogWriter {

    /**
     * Method to print
     */
    public void println(String msg);

    /**
     * initialization
     *
     */

    public boolean initialize(share.DescEntry entry, boolean active);

    /**
     * will mostly be used by outproducers to sum up
     * the information, maybe write them to a db
     */

    public boolean summary(share.DescEntry entry);


    /**
     * Returns the <CODE>Watcher</CODE> which is associated with this logger
     * @see share.Watcher
     * @return the associated <CODE>Watcher</CODE>
     */
    public Object getWatcher();

    /**
     * Set a <CODE>Watcher</CODE> to the <CODE>LogWriter</CODE>
     * This is usefull if a test starts a new office instance by itself. In this cases
     * the <CODE>LogWritter</CODE> could retrigger the <CODE>Watcher</CODE>
     * @see share.Watcher
     * @param watcher the new <CODE>Watcher</CODE>
     */
    public void setWatcher(Object watcher);

}
