/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package util.compare;

import convwatch.GraphicalDifferenceCheck;
import convwatch.GraphicalTestArguments;
import convwatch.DirectoryHelper;
import convwatch.FileHelper;

import lib.TestParameters;
import java.io.File;
import java.io.FileFilter;
import java.io.IOException;

import util.compare.DocComparator;
import convwatch.ConvWatchException;


class GraphicalComparator implements DocComparator
{
    GraphicalTestArguments m_aArguments;

    protected GraphicalComparator(TestParameters aParams)
        {
            m_aArguments = new GraphicalTestArguments(aParams);
        }

    /**
     * @return an instance of this object, but only it's interface
     */
    static DocComparator getInstance(TestParameters aParams)
        {
            // setting the default test parameter
            // TEST aParams
            GraphicalComparator a = new GraphicalComparator(aParams);
            return a;
        }

    /**
     * return a (FileFilter) function, which returns true, if the filename is a '*.prn' file
     */
    FileFilter getTrueIfPRNFile_FileFilter()
        {
            FileFilter aFileFilter = new FileFilter()
                {
                    public boolean accept( File pathname )
                        {
                            if (pathname.getName().endsWith(".prn"))
                            {
                                return true;
                            }
                            return false;
                        }
                };
            return aFileFilter;
        }

    /**
     * build a new file from _sEntry by
     * replacing the path equals to _sInputPath with _sReferencePath and replace it's suffix by _sNewSuffix.
     *  If _sInputPath is empty, replace the whole path by _sReferencePath.
     */
    protected String createSpecialFile(String _sEntry, String _sInputPath, String _sReferencePath, String _sNewSuffix)
        {
            String fs = System.getProperty("file.separator");
            String sNewSubDir = "";
            if (_sInputPath.length() > 0)
            {
                sNewSubDir = FileHelper.removeFirstDirectorysAndBasenameFrom(_sEntry, _sInputPath);
            }
            String sNameNoSuffix = FileHelper.getNameNoSuffix(FileHelper.getBasename(_sEntry));

            // add the sub path to the difference path
            String sNewReferencePath;
            if (sNewSubDir.length() > 0)
            {
                sNewReferencePath = _sReferencePath + fs + sNewSubDir;
            }
            else
            {
                sNewReferencePath = _sReferencePath;
            }
            // add the difference name
            sNewReferencePath += fs + sNameNoSuffix + _sNewSuffix;
            return sNewReferencePath;
        }

    boolean isReferenceOrDiffExistent(String _sNewSuffix)
        {
            boolean isExistent = false;

            // LLA? What if sReferencePath is a directory, but directory is empty? is the result then true or false;

            // wir muessen durch den InputPath durch und dann fuer jedes Dokument prufen, ob im angegebenen ReferencePath eine Reference existiert.
            String sInputPath = m_aArguments.getInputPath();
            if (FileHelper.isDir(sInputPath))
            {
                Object[] aList = DirectoryHelper.traverse(sInputPath, FileHelper.getFileFilter(), m_aArguments.includeSubDirectories());
                for (int i=0;i<aList.length;i++)
                {
                    // get document + path
                    String sEntry = (String)aList[i];
                    String sNewReferencePath = createSpecialFile(sEntry, sInputPath, m_aArguments.getReferencePath(), _sNewSuffix);
                    // split path from document path which only is equal to sInputPath (sub path)
                    if (FileHelper.exists(sNewReferencePath))
                    {
                        isExistent = true;
                    }
                }
            }
            else
            {
                // sInputPath is a file
                String sNewReferencePath = createSpecialFile(sInputPath, "", m_aArguments.getReferencePath(), _sNewSuffix);
                if (FileHelper.exists(sNewReferencePath))
                {
                    isExistent = true;
                }
            }
            return isExistent;
        }

    /**
     *  REFERENCE_PATH must set to directory/file, where the reference (*.prn files) (should) exist
     */
    public boolean isReferenceExistent()
        {
            return isReferenceOrDiffExistent(".prn");
        }

    /**
     *  INPUT_PATH must set, to directory/file, where the documents exist.
     *  REFERENCE_PATH must set to directory/file, where the created references (*.prn files) will create.
     */
    public void createReference() throws IOException
        {
            // woher kommt das TestDocument
            // INPUT_PATH
            // wohin
            // REFERENCE_PATH
            // mit was (Reference Application)
            // AppExecutionCmd
            try
            {
                String referenceInputPath = null;
                if(m_aArguments.getReferenceInputPath() == null)
                {
                    GraphicalDifferenceCheck.createReferences(m_aArguments.getInputPath(), m_aArguments.getReferencePath(), m_aArguments);
                }
                else
                {
                    referenceInputPath = m_aArguments.getReferenceInputPath();
                    GraphicalDifferenceCheck.createReferences(referenceInputPath, m_aArguments.getReferencePath(), m_aArguments);
                }
            }
            catch (ConvWatchException e)
            {
                // wrap it to IOException
                throw new java.io.IOException(e.getMessage());
            }
        }

    /**
     *  INPUT_PATH must set, to directory/file, where the documents exist.
     *  REFERENCE_PATH must set to directory/file, where the created references (*.prn files) will create.
     *  OUTPUT_PATH must set to a directory, there the whole ouptut will create
     */
    public boolean compare() throws IOException
        {
            try
            {
                if (FileHelper.isDebugEnabled())
                {
                    System.err.println("    Inputpath: '" + m_aArguments.getInputPath() + "'");
                    System.err.println("   Outputpath: '" + m_aArguments.getOutputPath() + "'");
                    System.err.println("Referencepath: '" + m_aArguments.getReferencePath() + "'");
                }
                return GraphicalDifferenceCheck.check(m_aArguments.getInputPath(), m_aArguments.getOutputPath(), m_aArguments.getReferencePath(), m_aArguments);
            }
            catch(ConvWatchException e)
            {
                // wrap it to IOException
                if (FileHelper.isDebugEnabled())
                {
                    System.err.println("Exception caught");
                    System.err.println("    Inputpath: '" + m_aArguments.getInputPath() + "'");
                    System.err.println("   Outputpath: '" + m_aArguments.getOutputPath() + "'");
                    System.err.println("Referencepath: '" + m_aArguments.getReferencePath() + "'");
                }
                throw new java.io.IOException(e.getMessage());
            }
        }

    /**
     *
     * INPUT_PATH must set to the original documents the directory structure is taken to see if the references exist in the DIFF_PATH
     * DIFF_PATH must set to the diff references
     */
    public boolean isDiffReferenceExistent() throws IOException
        {
            return isReferenceOrDiffExistent(".prn.diff0001.jpg");
        }

    /**
     *  INPUT_PATH must set, to directory/file, where the documents exist.
     *  REFERENCE_PATH must set to directory/file, where the created references (*.prn files) exists.
     *  OUTPUT_PATH must set to a directory, where the whole ouptut will create. Here the diffReference will create.
     *              At the momemt it's not possible to say only where the diffreferences will create.
     */
    public void createDiffReference() throws IOException
        {
            // this is the same like compareDiff(), but trash the result.
            compareDiff();
        }

    /**
     *  INPUT_PATH must set, to directory/file, where the documents exist.
     *  REFERENCE_PATH must set to directory/file, where the created references (*.prn files) exists.
     *  OUTPUT_PATH must set to a directory, where the whole ouptut will create.
     *  DIFF_PATH must set to a directory, where the older difference references exist, it's possible to set this to the same as REFERENCE_PATH
     *  but this is not the default and will not automatically set.
     */
    public boolean compareDiff() throws IOException
        {
            try
            {
                return GraphicalDifferenceCheck.check(m_aArguments.getInputPath(), m_aArguments.getOutputPath(), m_aArguments.getReferencePath(), m_aArguments.getDiffPath(), m_aArguments);
            }
            catch(ConvWatchException e)
            {
                // wrap it to IOException
                throw new java.io.IOException(e.getMessage());
            }
        }

}
