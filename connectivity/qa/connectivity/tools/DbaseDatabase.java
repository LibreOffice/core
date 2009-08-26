/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DbaseDatabase.java,v $
 * $Revision: 1.4.50.2 $
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
package connectivity.tools;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.XOfficeDatabaseDocument;
import com.sun.star.sdbc.SQLException;
import com.sun.star.uno.UnoRuntime;

import helper.URLHelper;
import java.io.File;

/**
 *
 * @author Ocke
 */
public class DbaseDatabase extends AbstractDatabase
{
    // --------------------------------------------------------------------------------------------------------

    public DbaseDatabase(final XMultiServiceFactory orb) throws Exception
    {
        super(orb);
        createDBDocument();
    }

    // --------------------------------------------------------------------------------------------------------
    public DbaseDatabase(final XMultiServiceFactory orb, final String _existingDocumentURL) throws Exception
    {
        super(orb, _existingDocumentURL);
    }

    /** creates an empty database document in a temporary location
     */
    private void createDBDocument() throws Exception
    {
        final File documentFile = File.createTempFile("dbase", ".odb");
        documentFile.deleteOnExit();
        final File subPath = new File(documentFile.getParent() + File.separator + documentFile.getName().replaceAll(".odb", "") + File.separator );
        subPath.mkdir();
        //subPath.deleteOnExit();
        m_databaseDocumentFile = URLHelper.getFileURLFromSystemPath(documentFile);
        final String path = URLHelper.getFileURLFromSystemPath(subPath.getPath());

        m_databaseDocument = (XOfficeDatabaseDocument) UnoRuntime.queryInterface(
                XOfficeDatabaseDocument.class, m_orb.createInstance("com.sun.star.sdb.OfficeDatabaseDocument"));
        m_dataSource = new DataSource(m_orb, m_databaseDocument.getDataSource());

        final XPropertySet dsProperties = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, m_databaseDocument.getDataSource());
        dsProperties.setPropertyValue("URL", "sdbc:dbase:" + path);

        final XStorable storable = (XStorable) UnoRuntime.queryInterface(XStorable.class, m_databaseDocument);
        storable.storeAsURL(m_databaseDocumentFile, new PropertyValue[]
                {
                });
    }

    /** drops the table with a given name

    @param _name
    the name of the table to drop
    @param _ifExists
    TRUE if it should be dropped only when it exists.
     */
    public void dropTable(final String _name,final boolean _ifExists) throws SQLException
    {
        String dropStatement = "DROP TABLE \"" + _name;
        executeSQL(dropStatement);
    }
}
