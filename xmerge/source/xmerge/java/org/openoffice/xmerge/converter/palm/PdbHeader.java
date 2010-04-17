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

package org.openoffice.xmerge.converter.palm;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

/**
 *  <p>Class used only internally by <code>PdbEncoder</code> and
 *  <code>PdbDecoder</code> to store, read and write a PDB header.</p>
 *
 *  <p>Note that fields are intended to be accessible only at the
 *  package level.</p>
 *
 *  <p>Some of the fields are internally represented using a
 *  larger type since Java does not have unsigned types.
 *  Some are not since they are not relevant for now.
 *  The <code>read</code> and <code>write</code> methods should
 *  handle them properly.</p>
 *
 *  @author  Herbie Ong
 *  @see     PalmDB
 *  @see     Record
 */
final class PdbHeader {


    /**  Name of the database. 32 bytes. */
    byte[] pdbName = null;

    /**
     *  Flags for the database.  Palm UInt16.  Unsignedness should be
     *  irrelevant.
     */
    short attribute = 0;

    /**  Application-specific version for the database.  Palm UInt16. */
    int version = 0;

    /**  Date created. Palm UInt32. */
    long creationDate = 0;

    /**  Date last modified. Palm UInt32.  */
    long modificationDate = 0;

    /**  Date last backup. Palm UInt32. */
    long lastBackupDate = 0;

    /**
     *  Incremented every time a <code>Record</code> is
     *  added, deleted or modified.  Palm UInt32.
     */
    long modificationNumber = 0;

    /**  Optional field. Palm UInt32. Unsignedness should be irrelevant. */
    int appInfoID = 0;

    /**  Optional field. Palm UInt32. Unsignedness should be irrelevant. */
    int sortInfoID = 0;

    /**  Database type ID. Palm UInt32. Unsignedness should be irrelevant. */
    int typeID = 0;

    /**  Database creator ID. Palm UInt32. Unsignedness should be irrelevant. */
    int creatorID = 0;

    /**  ??? */
    int uniqueIDSeed = 0;

    /**  See numRecords.  4 bytes. */
    int nextRecordListID = 0;

    /**
     *  Number of Records stored in the database header.
     *  If all the <code>Record</code> entries cannot fit in the header,
     *  then <code>nextRecordList</code> has the local ID of a
     *  RecordList that contains the next set of <code>Record</code>.
     *  Palm UInt16.
     */
    int numRecords = 0;


    /**
     *  Read in the data for the PDB header.  Need to
     *  preserve the unsigned value for some of the fields.
     *
     *  @param  di  A <code>DataInput</code> object.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    public void read(DataInput in) throws IOException {

        pdbName = new byte[PalmDB.NAME_LENGTH];
        in.readFully(pdbName);
        attribute = in.readShort();
        version = in.readUnsignedShort();
        creationDate = ((long) in.readInt()) & 0xffffffffL;
        modificationDate = ((long) in.readInt()) & 0xffffffffL;
        lastBackupDate = ((long) in.readInt())  & 0xffffffffL;
        modificationNumber = ((long) in.readInt()) & 0xffffffffL;
        appInfoID = in.readInt();
        sortInfoID = in.readInt();
        creatorID = in.readInt();
        typeID = in.readInt();
        uniqueIDSeed = in.readInt();
        nextRecordListID = in.readInt();
        numRecords = in.readUnsignedShort();
    }


    /**
     *  Write out PDB header data.
     *
     *  @param  out  A <code>DataOutput</code> object.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    public void write(DataOutput out) throws IOException {

        out.write(pdbName);
        out.writeShort(attribute);
        out.writeShort(version);
        out.writeInt((int) creationDate);
        out.writeInt((int) modificationDate);
        out.writeInt((int) lastBackupDate);
        out.writeInt((int) modificationNumber);
        out.writeInt(appInfoID);
        out.writeInt(sortInfoID);
        out.writeInt(typeID);
        out.writeInt(creatorID);
        out.writeInt(uniqueIDSeed);
        out.writeInt(nextRecordListID);
        out.writeShort(numRecords);
    }
}

