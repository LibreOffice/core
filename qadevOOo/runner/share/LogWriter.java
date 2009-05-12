/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: LogWriter.java,v $
 * $Revision: 1.4 $
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
