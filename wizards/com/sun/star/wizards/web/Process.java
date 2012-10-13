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
package com.sun.star.wizards.web;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import javax.xml.transform.Templates;
import javax.xml.transform.Transformer;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.w3c.dom.Document;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.ConfigSet;
import com.sun.star.wizards.common.FileAccess;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.common.UCB;
import com.sun.star.wizards.ui.event.Task;
import com.sun.star.wizards.web.data.CGContent;
import com.sun.star.wizards.web.data.CGDocument;
import com.sun.star.wizards.web.data.CGExporter;
import com.sun.star.wizards.web.data.CGLayout;
import com.sun.star.wizards.web.data.CGPublish;
import com.sun.star.wizards.web.data.CGSettings;
import com.sun.star.wizards.web.export.Exporter;

/**
 * This class is used to process a CGSession object
 * and generate a site. </br>
 * it does the following: <br/>
 * 1. create a temporary directory.<br/>
 * 2. export documents to the temporary directory.<br/>
 * 3. generate the TOC page, includes copying images from the
 * web wizard work directory and other layout files.<br/>
 * 4. publish, or copy, from the temporary directory to
 * different destinations.<br/>
 * 5. delete the temporary directory.<br/>
 * <br/>
 * to follow up the status/errors it uses a TaskListener object,
 * and an ErrorHandler. <br/>
 * in practice, the TaskListener is the status dialog,
 * and the Errorhandler does the interaction with the user,
 * if something goes wrong.<br/>
 * Note that this class takes it in count that
 * the given session object is prepared for it -
 * all preparations are done in WWD_Events.finishWizard methods.
 * <br/>
 * <br/>
 * 
 * note on error handling: <br/>
 * on "catch" clauses I tries to decide whether the
 * exception is fatal or not. For fatal exception an error message
 * is displayed (or rather: the errorHandler is being called...)
 * and a false is returned.
 * In less-fatal errors, the errorHandler "should decide" which means,
 * the user is given the option to "OK" or to "Cancel" and depending
 * on that interaction I cary on. 
 */
public class Process implements WebWizardConst, ProcessErrors
{

    private static final int TASKS_PER_DOC = 5;
    private static final int TASKS_PER_XSL = 2;
    private static final int TASKS_PER_PUBLISH = 2;
    private static final int TASKS_IN_PREPARE = 1;
    private static final int TASKS_IN_EXPORT = 2;
    private static final int TASKS_IN_GENERATE = 2;
    private static final int TASKS_IN_PUBLISH = 2;
    private static final int TASKS_IN_FINISHUP = 1;
    private CGSettings settings;
    private XMultiServiceFactory xmsf;
    private ErrorHandler errorHandler;
    private String tempDir;
    private FileAccess fileAccess;
    private UCB ucb;
    public Task myTask;
    /**
     * This is a cache for exporters, so I do not need to 
     * instanciate the same exporter more than once. 
     */
    private Map<CGExporter, Exporter> exporters = new HashMap<CGExporter, Exporter>(3);
    private boolean result;

    public Process(
            CGSettings settings,
            XMultiServiceFactory xmsf,
            ErrorHandler er)
            throws Exception
    {
        this.xmsf = xmsf;
        this.settings = settings;
        fileAccess = new FileAccess(xmsf);
        errorHandler = er;

        ucb = new UCB(xmsf);

        int taskSteps = getTaskSteps();
        myTask = new Task(TASK, TASK_PREPARE, taskSteps);

    }

    /**
     * @return to how many destinations should the
     * generated site be published. 
     */
    private int countPublish()
    {
        int count = 0;
        ConfigSet publishers = settings.cp_DefaultSession.cp_Publishing;
        for (int i = 0; i < publishers.getSize(); i++)
        {
            if (((CGPublish) publishers.getElementAt(i)).cp_Publish)
            {
                count++;
            }
        }
        return count;
    }

    /**
     * @return the number of task steps that this
     * session should have
     */
    private int getTaskSteps()
    {
        int docs = settings.cp_DefaultSession.cp_Content.cp_Documents.getSize();
        int xsl = 0;
        try
        {
            xsl = settings.cp_DefaultSession.getLayout().getTemplates(xmsf).size();
        }
        catch (Exception ex)
        {
        }
        int publish = countPublish();
        return
                TASKS_IN_PREPARE +
                TASKS_IN_EXPORT + docs * TASKS_PER_DOC +
                TASKS_IN_GENERATE + xsl * TASKS_PER_XSL +
                TASKS_IN_PUBLISH + publish * TASKS_PER_PUBLISH +
                TASKS_IN_FINISHUP;
    }

    /**
     * does the job
     */
    public void runProcess()
    {
        myTask.start();
        try
        {
            try
            {
                /*
                 * I use here '&&' so if one of the
                 * methods returns false, the next
                 * will not be called.
                 */
                result = createTempDir(myTask) && export(myTask) && generate(tempDir, myTask) && publish(tempDir, myTask);

            }
            finally
            {
                //cleanup must be called.
                result = result & cleanup(myTask);
            }
        }
        catch (Exception ex)
        {
            result = false;
        }

        if (!result)
        {
            myTask.fail();        //this is a bug protection. 
        }
        while (myTask.getStatus() < myTask.getMax())
        {
            myTask.advance(true);
        }
    }

    /**
     * creates a temporary directory.
     * @param task
     * @return true should continue
     */
    private boolean createTempDir(Task task)
    {

        tempDir = fileAccess.createNewDir(getSOTempDir(xmsf), "wwiztemp");
        if (tempDir == null)
        {
            error(null, null, ERROR_MKDIR, ErrorHandler.ERROR_PROCESS_FATAL);
            return false;
        }
        else
        {
            task.advance(true);
            return true;
        }
    }

    /**
     * @param xmsf
     * @return the staroffice /openoffice temporary directory
     */
    static String getSOTempDir(XMultiServiceFactory xmsf)
    {
        try
        {
            return FileAccess.getOfficePath(xmsf, "Temp", PropertyNames.EMPTY_STRING, PropertyNames.EMPTY_STRING);
        }
        catch (Exception e)
        {
        }
        return null;
    }

    // CLEANUP
    /**
     * delete the temporary directory
     * @return true should continue
     */
    private boolean cleanup(Task task)
    {

        task.setSubtaskName(TASK_FINISH);
        boolean b = fileAccess.delete(tempDir);
        if (!b)
        {
            error(null, null, ERROR_CLEANUP, ErrorHandler.ERROR_WARNING);
        }
        task.advance(b);
        return b;
    }

    /**
     * This method is used to copy style files to a target 
     * Directory: css and background.
     * Note that this method is static since it is
     * also used when displaying a "preview"
     */
    public static void copyMedia(UCB copy, CGSettings settings, String targetDir, Task task) throws Exception
    {

        //1. .css
        String sourceDir = FileAccess.connectURLs(settings.workPath, "styles");
        String filename = settings.cp_DefaultSession.getStyle().cp_CssHref;
        copy.copy(sourceDir, filename, targetDir, "style.css");

        task.advance(true);

        //2. background image
        String background = settings.cp_DefaultSession.cp_Design.cp_BackgroundImage;
        if (background != null && !background.equals(PropertyNames.EMPTY_STRING))
        {
            sourceDir = FileAccess.getParentDir(background);
            filename = background.substring(sourceDir.length());
            copy.copy(sourceDir, filename, targetDir + "/images", "background.gif");
        }

        task.advance(true);
    }

    /**
     * Copy "static" files (which are always the same, 
     * thus not user-input-dependant) to a target directory.
     * Note that this method is static since it is
     * also used when displaying a "preview"
     * @param copy
     * @param settings
     * @param targetDir
     * @throws Exception
     */
    public static void copyStaticImages(UCB copy, CGSettings settings, String targetDir)
            throws Exception
    {
        copy.copy(FileAccess.connectURLs(settings.workPath, "images"), targetDir + "/images");
    }

    /**
     * publish the given directory.
     * @param dir the source directory to publish from
     * @param task task tracking.
     * @return true if should continue
     */
    private boolean publish(String dir, Task task)
    {
        task.setSubtaskName(TASK_PUBLISH_PREPARE);
        ConfigSet set = settings.cp_DefaultSession.cp_Publishing;
        try
        {

            copyMedia(ucb, settings, dir, task);
            copyStaticImages(ucb, settings, dir);
            task.advance(true);
        }
        catch (Exception ex)
        {
            //error in copying media
            error(ex, PropertyNames.EMPTY_STRING, ERROR_PUBLISH_MEDIA, ErrorHandler.ERROR_PROCESS_FATAL);
            return false;
        }

        boolean result = true;

        for (int i = 0; i < set.getSize(); i++)
        {

            CGPublish p = (CGPublish) set.getElementAt(i);

            if (p.cp_Publish)
            {

                String key = (String) set.getKey(p);
                task.setSubtaskName(key);

                if (key.equals(ZIP_PUBLISHER))
                {
                    fileAccess.delete(p.cp_URL);
                }
                if (!publish(dir, p, ucb, task))
                {
                    return false;
                }

            }
        }

        return result;
    }

    /**
     * publish the given directory to the
     * given target CGPublish.
     * @param dir the dir to copy from
     * @param publish the object that specifies the target
     * @param copy ucb encapsulation
     * @param task task tracking
     * @return true if should continue
     */
    private boolean publish(String dir, CGPublish publish, UCB copy, Task task)
    {
        try
        {
            task.advance(true);
            copy.copy(dir, publish.url);
            task.advance(true);
            return true;
        }
        catch (Exception e)
        {
            task.advance(false);
            return error(e, publish, ERROR_PUBLISH, ErrorHandler.ERROR_NORMAL_IGNORE);
        }
    }
    /**
     * Generates the TOC pages for the current session.
     * @param targetDir generating to this directory.
     */
    public boolean generate(String targetDir, Task task)
    {
        boolean result = false;
        task.setSubtaskName(TASK_GENERATE_PREPARE);


        CGLayout layout = settings.cp_DefaultSession.getLayout();

        try
        {
            /*
             * here I create the DOM of the TOC to pass to the XSL 
             */
            Document doc = (Document) settings.cp_DefaultSession.createDOM();
            generate(xmsf, layout, doc, fileAccess, targetDir, task);

        }
        catch (Exception ex)
        {
            error(ex, PropertyNames.EMPTY_STRING, ERROR_GENERATE_XSLT, ErrorHandler.ERROR_PROCESS_FATAL);
            return false;
        }

        /* copy files which are not xsl from layout directory to 
         * website root.
         */
        try
        {

            task.setSubtaskName(TASK_GENERATE_COPY);

            copyLayoutFiles(ucb, fileAccess, settings, layout, targetDir);

            task.advance(true);

            result = true;
        }
        catch (Exception ex)
        {
            task.advance(false);
            return error(ex, null, ERROR_GENERATE_COPY, ErrorHandler.ERROR_NORMAL_ABORT);
        }



        return result;

    }

    /**
     * copies layout files which are not .xsl files
     * to the target directory.
     * @param ucb UCB encapsulatzion object
     * @param fileAccess filaAccess encapsulation object
     * @param settings web wizard settings
     * @param layout the layout object
     * @param targetDir the target directory to copy to
     * @throws Exception
     */
    public static void copyLayoutFiles(UCB ucb, FileAccess fileAccess, CGSettings settings, CGLayout layout, String targetDir)
            throws Exception
    {
        String filesPath = fileAccess.getURL(
                FileAccess.connectURLs(settings.workPath, "layouts/"), layout.cp_FSName);
        ucb.copy(filesPath, targetDir, new ExtensionVerifier("xsl"));

    }

    /**
     * generates the TOC page for the given layout.
     * This method might generate more than one file, depending
     * on how many .xsl files are in the
     * directory specifies by the given layout object.
     * @param xmsf
     * @param layout specifies the layout to use.
     * @param doc the DOM representation of the web wizard session
     * @param fileAccess encapsulation of FileAccess
     * @param targetPath target directory
     * @param task
     * @throws Exception
     */
    public static void generate(
            XMultiServiceFactory xmsf,
            CGLayout layout,
            Document doc,
            FileAccess fileAccess,
            String targetPath,
            Task task)
            throws Exception
    {
        /*
         * a map that contains xsl templates. the keys are the xsl file names. 
         */
        Map<String, Templates> templates = layout.getTemplates(xmsf);

        task.advance(true, TASK_GENERATE_XSL);

        /*
         * each template generates a page.
         */
        for (Iterator<String> i = templates.keySet().iterator(); i.hasNext();)
        {

            String key = PropertyNames.EMPTY_STRING;

            key = i.next();

            Transformer transformer = templates.get(key).newTransformer();

            doc.normalize();
            task.advance(true);

            /*
             * The target file name is like the xsl template filename
             * without the .xsl extension.
             */
            String fn = fileAccess.getPath(targetPath, key.substring(0, key.length() - 4));
            File f = new File(fn);
            FileOutputStream oStream = new FileOutputStream(f);
            // Due to a problem occuring when using Xalan-Java 2.6.0 and
            // Java 1.5.0, wrap f in a FileOutputStream here (otherwise, the
            // StreamResult's getSystemId would return a "file:/..." URL while
            // the Xalan code expects a "file:///..." URL):
            transformer.transform(
                    new DOMSource(doc), new StreamResult(oStream));
            oStream.close();
            task.advance(true);
        }
    }

    /**
     * I broke the export method to two methods
     * in a time where a tree with more than one contents was planned.
     * I left it that way, because it may be used in the future.
     * @param task
     * @return
     */
    private boolean export(Task task)
    {

        return export(settings.cp_DefaultSession.cp_Content, tempDir, task);

    }

    /**
     * This method could actually, with light modification, use recursion.
     * In the present situation, where we only use a "flat" list of
     * documents, instead of the original plan to use a tree,
     * the recursion is not implemented.
     * @param content the content ( directory-like, contains documents) 
     * @param dir (target directory for exporting this content.
     * @param task
     * @return true if should continue
     */
    private boolean export(CGContent content, String dir, Task task)
    {
        int toPerform = 1;
        String contentDir = dir;

        try
        {

            task.setSubtaskName(TASK_EXPORT_PREPARE);

            /* 1. create a content directory.
             * each content (at the moment there is only one :-( )
             * is created in its own directory.
             * faileure here is fatal.
             */
            contentDir = fileAccess.createNewDir(dir, content.cp_Name);
            if (contentDir == null || contentDir.equals(PropertyNames.EMPTY_STRING))
            {
                throw new IOException("Directory " + dir + " could not be created.");
            }
            content.dirName = FileAccess.getFilename(contentDir);

            task.advance(true, TASK_EXPORT_DOCUMENTS);
            toPerform--;

            /*2. export all documents and sub contents.
             * (at the moment, only documents, no subcontents)
             */
            Object item = null;
            for (int i = 0; i < content.cp_Documents.getSize(); i++)
            {
                try
                {
                    item = content.cp_Documents.getElementAt(i);
                    /*
                     * In present this is always the case.
                     * may be in the future, when
                     * a tree is used, it will be abit different.
                     */
                    if (item instanceof CGDocument)
                    {
                        if (!export((CGDocument) item, contentDir, task))
                        {
                            return false;
                        }
                    }
                    else /*
                     * we never get here since we
                     * did not implement sub-contents.
                     */ if (!export((CGContent) item, contentDir, task))
                    {
                        return false;
                    }
                }
                catch (SecurityException sx)
                {
                    // nonfatal 
                    if (!error(sx, item, ERROR_EXPORT_SECURITY, ErrorHandler.ERROR_NORMAL_IGNORE))
                    {
                        return false;
                    }
                    result = false;
                }
            }
        }
        catch (IOException iox)
        {
            //nonfatal
            return error(iox, content, ERROR_EXPORT_IO, ErrorHandler.ERROR_NORMAL_IGNORE);

        }
        catch (SecurityException se)
        {
            //nonfatal
            return error(se, content, ERROR_EXPORT_SECURITY, ErrorHandler.ERROR_NORMAL_IGNORE);
        }
        failTask(task, toPerform);
        return true;

    }

    /**
     * exports a single document
     * @param doc the document to export
     * @param dir the target directory
     * @param task task tracking
     * @return true if should continue
     */
    private boolean export(CGDocument doc, String dir, Task task)
    {

        //first I check if the document was already validated...
        if (!doc.valid)
        {
            try
            {
                doc.validate(xmsf, null);
            }
            catch (Exception ex)
            {
                //fatal
                error(ex, doc, ERROR_DOC_VALIDATE, ErrorHandler.ERROR_PROCESS_FATAL);
                return false;
            }
        //get the exporter specified for this document  
        }
        CGExporter exporter = (CGExporter) settings.cp_Exporters.getElement(doc.cp_Exporter);


        try
        {

            /*
             * here I calculate the destination filename. 
             * I take the original filename (docFilename), substract the extension, (docExt) -> (fn) 
             * and find an available filename which starts with
             * this filename, but with the new extension. (destExt)
             */
            String docFilename = FileAccess.getFilename(doc.cp_URL);

            String docExt = FileAccess.getExtension(docFilename);
            String fn = doc.localFilename.substring(0, doc.localFilename.length() - docExt.length() - 1); //filename without extension 

            /*
             * the copyExporter does not change
             * the extension of the target...
             */
            String destExt = (exporter.cp_Extension.equals(PropertyNames.EMPTY_STRING)
                    ? FileAccess.getExtension(docFilename)
                    : exporter.cp_Extension);

            /* if this filter needs to export to its own directory...
             * this is the case in, for example, impress html export
             */
            if (exporter.cp_OwnDirectory)
            { //+++
                dir = fileAccess.createNewDir(dir, fn);
                doc.dirName = FileAccess.getFilename(dir);
            }

            /*
             * if two files with the same name 
             * need to be exported ? So here
             * i get a new filename, so I do not
             * overwrite files...
             */
            String file = fileAccess.getNewFile(dir, fn, destExt);


            /* set filename with extension.
             * this will be used by the exporter,
             * and to generate the TOC.
             */
            doc.urlFilename = FileAccess.getFilename(file);

            task.advance(true);

            try
            {
                //export
                getExporter(exporter).export(doc, file, xmsf, task);
                task.advance(true);
            }
            /*
             * getExporter(..) throws 
             * IllegalAccessException, InstantiationException, ClassNotFoundException
             * export() throws Exception 
             */
            catch (Exception ex)
            {
                //nonfatal
                if (!error(ex, doc, ERROR_EXPORT, ErrorHandler.ERROR_NORMAL_IGNORE))
                {
                    return false;
                }
            }
        }
        catch (Exception ex)
        {
            //nonfatal
            if (!error(ex, doc, ERROR_EXPORT_MKDIR, ErrorHandler.ERROR_NORMAL_ABORT))
            {
                return false;
            }
        }

        return true;

    }

    /**
     * submit an error.
     * @param ex the exception 
     * @param arg1 error argument
     * @param arg2 error argument 2
     * @param errType error type
     * @return the interaction result
     */
    private boolean error(Exception ex, Object arg1, int arg2, int errType)
    {
        result = false;
        return errorHandler.error(ex, arg1, arg2, errType);
    }

    /**
     * advances the given task in the given count of steps,
     * marked as failed.
     * @param task the task to advance
     * @param count the number of steps to advance
     */
    private void failTask(Task task, int count)
    {
        while (count-- > 0)
        {
            task.advance(false);
        }
    }

    /**
     * creates an instance of the exporter class 
     * as specified by the
     * exporter object.
     * @param export specifies the exporter to be created
     * @return the Exporter instance
     * @throws ClassNotFoundException
     * @throws IllegalAccessException
     * @throws InstantiationException
     */
    private Exporter createExporter(CGExporter export)
            throws ClassNotFoundException,
            IllegalAccessException,
            InstantiationException
    {
        Exporter e = (Exporter) Class.forName(export.cp_ExporterClass).newInstance();
        e.init(export);
        return e;
    }

    /**
     * searches the an exporter for the given CGExporter object
     * in the cache.
     * If its not there, creates it, stores it in the cache and 
     * returns it.
     * @param export specifies the needed exporter.
     * @return an Exporter instance
     * @throws ClassNotFoundException thrown when using Class.forName(string)
     * @throws IllegalAccessException thrown when using Class.forName(string)
     * @throws InstantiationException thrown when using Class.forName(string)
     */
    private Exporter getExporter(CGExporter export)
            throws ClassNotFoundException,
            IllegalAccessException,
            InstantiationException
    {
        Exporter exp = exporters.get(export);
        if (exp == null)
        {
            exp = createExporter(export);
            exporters.put(export, exp);
        }
        return exp;
    }

    /**
     * @return tru if everything went smooth, false
     * if error(s) accured.
     */
    public boolean getResult()
    {
        return (myTask.getFailed() == 0) && result;
    }
}
