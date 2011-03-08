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

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

/**
 *  <p>Class used only internally by <code>PDBEncoder</code> and
 *  <code>PDBDecoder</code> to store, read and write a pdb header.</p>
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
 *  @author    Herbie Ong
 */

final class PDBHeader {

    /** name of the database. 32 bytes. */
    byte[] pdbName = null;

    /** flags for the database. Palm UInt16. Unsignedness should be irrelevant. */
    short attribute = 0;

    /** application-specific version for the database. Palm UInt16 */
    int version = 0;

    /** date created. Palm UInt32 */
    long creationDate = 0;

    /** date last modified. Palm UInt32  */
    long modificationDate = 0;

    /** date last backup. Palm UInt32 */
    long lastBackupDate = 0;

    /**
     *  incremented every time a record is
     *  added, deleted or modified.  Palm UInt32.
     */
    long modificationNumber = 0;

    /** optional field. Palm UInt32. Unsignedness should be irrelevant. */
    int appInfoID = 0;

    /** optional field. Palm UInt32. Unsignedness should be irrelevant. */
    int sortInfoID = 0;

    /** database type id. Palm UInt32. Unsignedness should be irrelevant. */
    int typeID = 0;

    /** database creator id. Palm UInt32. Unsignedness should be irrelevant. */
    int creatorID = 0;

    /** ??? */
    int uniqueIDSeed = 0;

    /** see numRecords. 4 bytes. */
    int nextRecordListID = 0;

    /**
     *  number of records stored in the database header.
     *  If all the record entries cannot fit in the header,
     *  then nextRecordList has the local ID of a
     *  recordList that contains the next set of records.
     *  Palm UInt16.
     */
    int numRecords = 0;

    /**
     *  Read in the data for the pdb header.  Need to
     *  preserve the unsigned value for some of the fields.
     *
     *  @param   di    a DataInput object
     *  @throws   IOException    if I/O error occurs
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
     *  Write out pdb header data.
     *
     *  @param   out    a DataOut object
     *  @throws   IOException    if I/O error occurs
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

