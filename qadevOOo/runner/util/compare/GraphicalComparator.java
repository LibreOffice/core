/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: GraphicalComparator.java,v $
 * $Revision: 1.6 $
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

// -----------------------------------------------------------------------------
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
                Object[] aList = DirectoryHelper.traverse(sInputPath, m_aArguments.getFileFilter(), m_aArguments.includeSubDirectories());
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
