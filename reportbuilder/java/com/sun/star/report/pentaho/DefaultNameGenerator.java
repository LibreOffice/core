/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DefaultNameGenerator.java,v $
 * $Revision: 1.5 $
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
package com.sun.star.report.pentaho;

import java.io.IOException;

import com.sun.star.report.OutputRepository;

public class DefaultNameGenerator
{

    private final OutputRepository outputRepository;

    public DefaultNameGenerator(final OutputRepository outputRepository)
    {
        if (outputRepository == null)
        {
            throw new NullPointerException();
        }
        this.outputRepository = outputRepository;
    }

    public String generateName(final String namePrefix, final String mimeType)
            throws IOException
    {
        return generateName(namePrefix, mimeType, true);
    }

    public String generateStorageName(final String namePrefix, final String mimeType)
            throws IOException
    {
        return generateName(namePrefix, mimeType, false);
    }

    /**
     * Generates a new, unique name for storing resources in the output repository. Assuming that proper synchronization
     * has been applied, the generated name will be unique within that repository.
     *
     * @param namePrefix a user defined name for that resource.
     * @param mimeType   the mime type of the resource to be stored in the repository.
     * @return the generated, fully qualified name.
     */
    private String generateName(final String namePrefix, final String mimeType, final boolean isStream)
            throws IOException
    {
        final String name;
        if (namePrefix != null)
        {
            name = namePrefix;
        }
        else
        {
            name = "file";
        }

        StringBuffer firstFileName = new StringBuffer();
        firstFileName.append(name);
        final String suffix;
        if (mimeType != null)
        {
            suffix = getSuffixForType(mimeType);
            firstFileName.append('.');
            firstFileName.append(suffix);
        }
        else
        {
            suffix = null;
        }
        String newName = firstFileName.toString();
        boolean exists;
        if (isStream)
        {
            exists = outputRepository.exists(newName);
        }
        else
        {
            exists = outputRepository.existsStorage(newName);
        }
        if (exists)
        {
            int counter = 0;
            while (exists)
            {
                if (counter < 0) // wraparound should not happen..
                {
                    throw new IOException();
                }
                firstFileName.delete(0,firstFileName.length() );
                firstFileName.append(name);
                firstFileName.append(counter);
                if (suffix != null)
                {
                    firstFileName.append('.');
                    firstFileName.append(suffix);
                }
                newName = firstFileName.toString();
                if (isStream)
                {
                    exists = outputRepository.exists(newName);
                }
                else
                {
                    exists = outputRepository.existsStorage(newName);
                }
                counter++;
            }
        }
        return newName;
    }

    protected String getSuffixForType(final String mimeType)
    {
        if ("image/png".equals(mimeType))
        {
            return "png";
        }
        if ("image/jpeg".equals(mimeType))
        {
            return "jpg";
        }
        if ("image/gif".equals(mimeType))
        {
            return "gif";
        }

        // todo ... use a flexible mapping ...
        return "dat";
    }
}
