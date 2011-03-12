
/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 *
 * Copyright 1997-2008 Sun Microsystems, Inc. All rights reserved.
 *
 * The contents of this file are subject to the terms of either the GNU
 * General Public License Version 2 only ("GPL") or the Common Development
 * and Distribution License("CDDL") (collectively, the "License").  You
 * may not use this file except in compliance with the License. You can obtain
 * a copy of the License at https://glassfish.dev.java.net/public/CDDL+GPL.html
 * or glassfish/bootstrap/legal/LICENSE.txt.  See the License for the specific
 * language governing permissions and limitations under the License.
 *
 * When distributing the software, include this License Header Notice in each
 * file and include the License file at glassfish/bootstrap/legal/LICENSE.txt.
 * Sun designates this particular file as subject to the "Classpath" exception
 * as provided by Sun in the GPL Version 2 section of the License file that
 * accompanied this code.  If applicable, add the following below the License
 * Header, with the fields enclosed by brackets [] replaced by your own
 * identifying information: "Portions Copyrighted [year]
 * [name of copyright owner]"
 *
 * Contributor(s):
 *
 * If you wish your version of this file to be governed by only the CDDL or
 * only the GPL Version 2, indicate your decision by adding "[Contributor]
 * elects to include this software in this distribution under the [CDDL or GPL
 * Version 2] license."  If you don't indicate a single choice of license, a
 * recipient has the option to distribute your version of this file under
 * either the CDDL, the GPL Version 2 or to extend the choice of license to
 * its licensees as provided above.  However, if you add GPL Version 2 code
 * and therefore, elected the GPL Version 2 license, then the option applies
 * only if the new code is made subject to such option by the copyright
 * holder.
 */

package com.sun.star.servicetag;

import java.io.*;
import java.net.URL;
import java.util.Collection;
import java.util.Map;
import java.util.Set;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;
import org.xml.sax.InputSource;

import javax.xml.XMLConstants;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.validation.Schema;
import javax.xml.validation.SchemaFactory;
import javax.xml.validation.Validator;

// For write operation
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

/**
 * XML Support Class for Product Registration.
 */
class RegistrationDocument {

    private static final String REGISTRATION_DATA_SCHEMA =
            "/com/sun/star/servicetag/resources/product_registration.xsd";
    private static final String REGISTRATION_DATA_VERSION = "1.0";
    private static final String SERVICE_TAG_VERSION = "1.0";
    final static String ST_NODE_REGISTRATION_DATA = "registration_data";
    final static String ST_ATTR_REGISTRATION_VERSION = "version";
    final static String ST_NODE_ENVIRONMENT = "environment";
    final static String ST_NODE_HOSTNAME = "hostname";
    final static String ST_NODE_HOST_ID = "hostId";
    final static String ST_NODE_OS_NAME = "osName";
    final static String ST_NODE_OS_VERSION = "osVersion";
    final static String ST_NODE_OS_ARCH = "osArchitecture";
    final static String ST_NODE_SYSTEM_MODEL = "systemModel";
    final static String ST_NODE_SYSTEM_MANUFACTURER = "systemManufacturer";
    final static String ST_NODE_CPU_MANUFACTURER = "cpuManufacturer";
    final static String ST_NODE_SERIAL_NUMBER = "serialNumber";
    final static String ST_NODE_PHYS_MEM = "physmem";
    final static String ST_NODE_CPU_INFO = "cpuinfo";
    final static String ST_NODE_SOCKETS = "sockets";
    final static String ST_NODE_CORES = "cores";
    final static String ST_NODE_VIRT_CPUS = "virtcpus";
    final static String ST_NODE_CPU_NAME = "name";
    final static String ST_NODE_CLOCK_RATE = "clockrate";
    final static String ST_NODE_REGISTRY = "registry";
    final static String ST_ATTR_REGISTRY_URN = "urn";
    final static String ST_ATTR_REGISTRY_VERSION = "version";
    final static String ST_NODE_SERVICE_TAG = "service_tag";
    final static String ST_NODE_INSTANCE_URN = "instance_urn";
    final static String ST_NODE_PRODUCT_NAME = "product_name";
    final static String ST_NODE_PRODUCT_VERSION = "product_version";
    final static String ST_NODE_PRODUCT_URN = "product_urn";
    final static String ST_NODE_PRODUCT_PARENT_URN = "product_parent_urn";
    final static String ST_NODE_PRODUCT_PARENT = "product_parent";
    final static String ST_NODE_PRODUCT_DEFINED_INST_ID = "product_defined_inst_id";
    final static String ST_NODE_PRODUCT_VENDOR = "product_vendor";
    final static String ST_NODE_PLATFORM_ARCH = "platform_arch";
    final static String ST_NODE_TIMESTAMP = "timestamp";
    final static String ST_NODE_CONTAINER = "container";
    final static String ST_NODE_SOURCE = "source";
    final static String ST_NODE_INSTALLER_UID = "installer_uid";

    static RegistrationData load(InputStream in) throws IOException {
        Document document = initializeDocument(in);

        // Gets the registration URN
        Element root = getRegistrationDataRoot(document);
        Element registryRoot =
                getSingletonElementFromRoot(root, ST_NODE_REGISTRY);
        String urn = registryRoot.getAttribute(ST_ATTR_REGISTRY_URN);

        // Construct a new RegistrationData object from the DOM tree
        // Initialize the environment map and service tags
        RegistrationData regData = new RegistrationData(urn);
        addServiceTags(registryRoot, regData);

        Element envRoot = getSingletonElementFromRoot(root, ST_NODE_ENVIRONMENT);
        buildEnvironmentMap(envRoot, regData);

        Element cpuInfo = getSingletonElementFromRoot(envRoot, ST_NODE_CPU_INFO);
        buildCpuInfoMap(cpuInfo, regData);
        return regData;
    }

    static void store(OutputStream os, RegistrationData registration)
            throws IOException {
        // create a new document with the root node
        Document document = initializeDocument();

        // create the nodes for the environment map and the service tags
        // in the registration data
        addEnvironmentNodes(document,
                            registration.getEnvironmentMap(),
                            registration.getCpuInfoMap());
        addServiceTagRegistry(document,
                              registration.getRegistrationURN(),
                              registration.getServiceTags());
        transform(document, os);
    }

    // initialize a document from an input stream
    private static Document initializeDocument(InputStream in) throws IOException {
        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        SchemaFactory sf = null;
        try {
            // Some Java versions (e.g., 1.5.0_06-b05) fail with a
            // NullPointerException if SchemaFactory.newInstance is called with
            // a null context class loader, so work around that here (and the
            // class loader of this class hopefully is not the null bootstrap
            // class loader):
            ClassLoader cl = Thread.currentThread().getContextClassLoader();
            if (cl == null) {
                Thread.currentThread().setContextClassLoader(
                    RegistrationDocument.class.getClassLoader());
            }
            try {
               sf = SchemaFactory.newInstance(
                   XMLConstants.W3C_XML_SCHEMA_NS_URI);
            } finally {
               Thread.currentThread().setContextClassLoader(cl);
            }

            Schema schema = null;
            try {
                // Even using the workaround above is not enough on some
                // Java versions. Therefore try to workaround the validation
                // completely!
                URL xsdUrl = RegistrationDocument.class.getResource(REGISTRATION_DATA_SCHEMA);
                schema = sf.newSchema(xsdUrl);
            }
        catch (NullPointerException nex) {
        }

            Validator validator = null;
            if (schema != null)
                validator = schema.newValidator();

            DocumentBuilder builder = factory.newDocumentBuilder();
            Document doc = builder.parse(new InputSource(in));

            if (validator != null)
                validator.validate(new DOMSource(doc));

            return doc;
        } catch (SAXException sxe) {
            IllegalArgumentException e = new IllegalArgumentException("Error generated in parsing");
            e.initCause(sxe);
            throw e;
        } catch (ParserConfigurationException pce) {
            // Parser with specific options can't be built
            // should not reach here
            InternalError x = new InternalError("Error in creating the new document");
            x.initCause(pce);
            throw x;
        }
    }

    // initialize a new document for the registration data
    private static Document initializeDocument() throws IOException {
        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        try {
            DocumentBuilder builder = factory.newDocumentBuilder();
            Document doc = builder.newDocument();

            // initialize the document with the registration_data root
            Element root = doc.createElement(ST_NODE_REGISTRATION_DATA);
            doc.appendChild(root);
            root.setAttribute(ST_ATTR_REGISTRATION_VERSION, REGISTRATION_DATA_VERSION);

            return doc;
        } catch (ParserConfigurationException pce) {
            // Parser with specified options can't be built
            // should not reach here
            InternalError x = new InternalError("Error in creating the new document");
            x.initCause(pce);
            throw x;
        }
    }

    // Transform the current DOM tree with the given output stream.
    private static void transform(Document document, OutputStream os) {
        try {
            // Use a Transformer for output
            TransformerFactory tFactory = TransformerFactory.newInstance();
            tFactory.setAttribute("indent-number", new Integer(3));

            Transformer transformer = tFactory.newTransformer();

            transformer.setOutputProperty(OutputKeys.INDENT, "yes");
            transformer.setOutputProperty(OutputKeys.METHOD, "xml");
            transformer.setOutputProperty(OutputKeys.ENCODING, "UTF-8");
            transformer.setOutputProperty(OutputKeys.STANDALONE, "yes");
            transformer.transform(new DOMSource(document),
                new StreamResult(new BufferedWriter(new OutputStreamWriter(os, "UTF-8"))));
        } catch (UnsupportedEncodingException ue) {
            // Should not reach here
            InternalError x = new InternalError("Error generated during transformation");
            x.initCause(ue);
            throw x;
        } catch (TransformerConfigurationException tce) {
            // Error generated by the parser
            // Should not reach here
            InternalError x = new InternalError("Error in creating the new document");
            x.initCause(tce);
            throw x;
        } catch (TransformerException te) {
            // Error generated by the transformer
            InternalError x = new InternalError("Error generated during transformation");
            x.initCause(te);
            throw x;
        }
    }

    private static void addServiceTagRegistry(Document document,
                                              String registryURN,
                                              Set<ServiceTag> svcTags) {
        // add service tag registry node and its attributes
        Element reg = document.createElement(ST_NODE_REGISTRY);
        reg.setAttribute(ST_ATTR_REGISTRY_URN, registryURN);
        reg.setAttribute(ST_ATTR_REGISTRY_VERSION, SERVICE_TAG_VERSION);

        Element root = getRegistrationDataRoot(document);
        root.appendChild(reg);

        // adds the elements for the service tags
        for (ServiceTag st : svcTags) {
            addServiceTagElement(document, reg, st);
        }
    }

    private static void addServiceTagElement(Document document,
                                             Element registryRoot,
                                             ServiceTag st) {
        Element svcTag = document.createElement(ST_NODE_SERVICE_TAG);
        registryRoot.appendChild(svcTag);
        addChildElement(document, svcTag,
                        ST_NODE_INSTANCE_URN, st.getInstanceURN());
        addChildElement(document, svcTag,
                        ST_NODE_PRODUCT_NAME, st.getProductName());
        addChildElement(document, svcTag,
                        ST_NODE_PRODUCT_VERSION, st.getProductVersion());
        addChildElement(document, svcTag,
                        ST_NODE_PRODUCT_URN, st.getProductURN());
        addChildElement(document, svcTag,
                        ST_NODE_PRODUCT_PARENT_URN, st.getProductParentURN());
        addChildElement(document, svcTag,
                        ST_NODE_PRODUCT_PARENT, st.getProductParent());
        addChildElement(document, svcTag,
                        ST_NODE_PRODUCT_DEFINED_INST_ID,
                        st.getProductDefinedInstanceID());
        addChildElement(document, svcTag,
                        ST_NODE_PRODUCT_VENDOR, st.getProductVendor());
        addChildElement(document, svcTag,
                        ST_NODE_PLATFORM_ARCH, st.getPlatformArch());
        addChildElement(document, svcTag,
                        ST_NODE_TIMESTAMP, Util.formatTimestamp(st.getTimestamp()));
        addChildElement(document, svcTag,
                        ST_NODE_CONTAINER, st.getContainer());
        addChildElement(document, svcTag,
                        ST_NODE_SOURCE, st.getSource());
        addChildElement(document, svcTag,
                        ST_NODE_INSTALLER_UID,
                        String.valueOf(st.getInstallerUID()));
    }

    private static void addChildElement(Document document, Element root,
                                        String element, String text) {
        Element node = document.createElement(element);
        node.appendChild(document.createTextNode(text));
        root.appendChild(node);
    }

    // Constructs service tags from the document
    private static void addServiceTags(Element registryRoot,
                                       RegistrationData registration) {
        NodeList children = registryRoot.getElementsByTagName(ST_NODE_SERVICE_TAG);
        int length = (children == null ? 0 : children.getLength());
        for (int i = 0; i < length; i++) {
            Element svcTagElement = (Element) children.item(i);
            ServiceTag st = getServiceTag(svcTagElement);
            registration.addServiceTag(st);
        }
    }

    // build environment map from the document
    private static void buildEnvironmentMap(Element envRoot,
                                         RegistrationData registration) {
        registration.setEnvironment(ST_NODE_HOSTNAME, getTextValue(envRoot, ST_NODE_HOSTNAME));
        registration.setEnvironment(ST_NODE_HOST_ID, getTextValue(envRoot, ST_NODE_HOST_ID));
        registration.setEnvironment(ST_NODE_OS_NAME, getTextValue(envRoot, ST_NODE_OS_NAME));
        registration.setEnvironment(ST_NODE_OS_VERSION, getTextValue(envRoot, ST_NODE_OS_VERSION));
        registration.setEnvironment(ST_NODE_OS_ARCH, getTextValue(envRoot, ST_NODE_OS_ARCH));
        registration.setEnvironment(ST_NODE_SYSTEM_MODEL, getTextValue(envRoot, ST_NODE_SYSTEM_MODEL));
        registration.setEnvironment(ST_NODE_SYSTEM_MANUFACTURER, getTextValue(envRoot, ST_NODE_SYSTEM_MANUFACTURER));
        registration.setEnvironment(ST_NODE_CPU_MANUFACTURER, getTextValue(envRoot, ST_NODE_CPU_MANUFACTURER));
        registration.setEnvironment(ST_NODE_SERIAL_NUMBER, getTextValue(envRoot, ST_NODE_SERIAL_NUMBER));
        registration.setEnvironment(ST_NODE_PHYS_MEM, getTextValue(envRoot, ST_NODE_PHYS_MEM));
    }

    private static void buildCpuInfoMap(Element cpuInfoRoot,
                                         RegistrationData registration) {
        registration.setCpuInfo(ST_NODE_SOCKETS, getTextValue(cpuInfoRoot, ST_NODE_SOCKETS));
        registration.setCpuInfo(ST_NODE_CORES, getTextValue(cpuInfoRoot, ST_NODE_CORES));
        registration.setCpuInfo(ST_NODE_VIRT_CPUS, getTextValue(cpuInfoRoot, ST_NODE_VIRT_CPUS));
        registration.setCpuInfo(ST_NODE_CPU_NAME, getTextValue(cpuInfoRoot, ST_NODE_CPU_NAME));
        registration.setCpuInfo(ST_NODE_CLOCK_RATE, getTextValue(cpuInfoRoot, ST_NODE_CLOCK_RATE));
    }

    // add the nodes representing the environment map in the document
    private static void addEnvironmentNodes(Document document,
                                            Map<String, String> envMap,
                                            Map<String, String> cpuInfoMap) {
        Element root = getRegistrationDataRoot(document);

        Element env = document.createElement(ST_NODE_ENVIRONMENT);
        root.appendChild(env);
        Set<Map.Entry<String, String>> keys = envMap.entrySet();
        for (Map.Entry<String, String> entry : keys) {
            addChildElement(document, env, entry.getKey(), entry.getValue());
        }

        Element cpuInfo = document.createElement(ST_NODE_CPU_INFO);
        env.appendChild(cpuInfo);
        keys = cpuInfoMap.entrySet();
        for (Map.Entry<String, String> entry : keys) {
            addChildElement(document, cpuInfo, entry.getKey(), entry.getValue());
        }
    }

    private static Element getRegistrationDataRoot(Document doc) {
        Element root = doc.getDocumentElement();
        if (!root.getNodeName().equals(ST_NODE_REGISTRATION_DATA)) {
            throw new IllegalArgumentException("Not a " +
                    ST_NODE_REGISTRATION_DATA +
                    " node \"" + root.getNodeName() + "\"");
        }
        return root;
    }

    private static Element getSingletonElementFromRoot(Element root, String name) {
        NodeList children = root.getElementsByTagName(name);
        int length = (children == null ? 0 : children.getLength());
        if (length != 1) {
            throw new IllegalArgumentException("Invalid number of " + name +
                    " nodes = " + length);
        }
        Element e = (Element) children.item(0);
        if (!e.getNodeName().equals(name)) {
            throw new IllegalArgumentException("Not a  " + name +
                    " node \"" + e.getNodeName() + "\"");
        }
        return e;
    }

    // Constructs one ServiceTag instance from a service tag element root
    private static ServiceTag getServiceTag(Element svcTagElement) {
        return new ServiceTag(
            getTextValue(svcTagElement, ST_NODE_INSTANCE_URN),
            getTextValue(svcTagElement, ST_NODE_PRODUCT_NAME),
            getTextValue(svcTagElement, ST_NODE_PRODUCT_VERSION),
            getTextValue(svcTagElement, ST_NODE_PRODUCT_URN),
            getTextValue(svcTagElement, ST_NODE_PRODUCT_PARENT),
            getTextValue(svcTagElement, ST_NODE_PRODUCT_PARENT_URN),
            getTextValue(svcTagElement, ST_NODE_PRODUCT_DEFINED_INST_ID),
            getTextValue(svcTagElement, ST_NODE_PRODUCT_VENDOR),
            getTextValue(svcTagElement, ST_NODE_PLATFORM_ARCH),
            getTextValue(svcTagElement, ST_NODE_CONTAINER),
            getTextValue(svcTagElement, ST_NODE_SOURCE),
            Util.getIntValue(getTextValue(svcTagElement, ST_NODE_INSTALLER_UID)),
            Util.parseTimestamp(getTextValue(svcTagElement, ST_NODE_TIMESTAMP))
        );
    }

    private static String getTextValue(Element e, String tagName) {
        String value = "";
        NodeList nl = e.getElementsByTagName(tagName);
        if (nl != null && nl.getLength() > 0) {
            Element el = (Element) nl.item(0);
            Node node = el.getFirstChild();
            if (node != null) {
                value = node.getNodeValue();
            }
        }
        return value;
    }
}
