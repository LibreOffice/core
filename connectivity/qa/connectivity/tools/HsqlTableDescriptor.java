/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: HsqlTableDescriptor.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2006-02-06 16:43:25 $
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

package connectivity.tools;

/** is a very simply descriptor of a HSQL table, to be used with a HsqlDatabase.createTable method
 */
public class HsqlTableDescriptor
{
    private String                  m_name;
    private HsqlColumnDescriptor[]  m_columns;

    /** Creates a new instance of HsqlTableDescriptor */
    public HsqlTableDescriptor( String _name, HsqlColumnDescriptor[] _columns )
    {
        m_name = _name;
        m_columns = _columns;
    }

    /** returns the name of the table
     */
    public String getName()
    {
        return m_name;
    }

    /** returns the set of column descriptors for the table
     */
    public HsqlColumnDescriptor[] getColumns()
    {
        return m_columns;
    }
}
