/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DefaultNameGenerator.java,v $
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
package com.sun.star.report.pentaho;

import java.io.IOException;

import com.sun.star.report.OutputRepository;

public class DefaultNameGenerator
{

    private OutputRepository outputRepository;

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
    private String generateName(final String namePrefix, final String mimeType, boolean isStream)
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

        String firstFileName = name;
        String suffix = null;
        if (mimeType != null)
        {
            suffix = getSuffixForType(mimeType);
            firstFileName += "." + suffix;
        }
        boolean exists = false;
        if (isStream)
        {
            exists = outputRepository.exists(firstFileName);
        }
        else
        {
            exists = outputRepository.existsStorage(firstFileName);
        }
        if (exists == false)
        {
            return firstFileName;
        }
        int counter = 0;
        while (true)
        {
            if (counter < 0) // wraparound should not happen..
            {
                throw new IOException();
            }
            String filename = name + counter;
            if (suffix != null)
            {
                filename += "." + suffix;
            }
            if (isStream)
            {
                exists = outputRepository.exists(filename);
            }
            else
            {
                exists = outputRepository.existsStorage(filename);
            }
            if (exists == false)
            {
                return filename;
            }
            counter += 1;
        }
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
