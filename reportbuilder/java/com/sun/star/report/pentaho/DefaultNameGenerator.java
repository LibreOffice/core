/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


package com.sun.star.report.pentaho;

import com.sun.star.report.OutputRepository;

import java.io.IOException;


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
     * @param isStream
     * @return the generated, fully qualified name.
     * @throws java.io.IOException
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
                firstFileName.delete(0, firstFileName.length());
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
