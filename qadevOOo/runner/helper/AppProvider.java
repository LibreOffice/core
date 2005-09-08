/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AppProvider.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:17:35 $
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

package helper;

/**
 *
 * Interface to get a Manager to access the application to check
 */
public interface AppProvider {

    /**
     * Method to get the desired Manager
     */
    public Object getManager(lib.TestParameters param);

    /**
     * Method to dispose the desired Manager
     */
    public boolean disposeManager(lib.TestParameters param);

    /**
     * Close an office.
     * @param param The test parameters.
     * @param closeIfPossible If true, close even if
     * it was running before the test
     * @return True, if close worked.
     */
    public boolean closeExistingOffice(lib.TestParameters param,
                                                boolean closeIfPossible);
}
