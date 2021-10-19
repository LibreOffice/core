#!/usr/bin/python

import sys, getopt, os, shutil
from typing import Type
import xml.etree.ElementTree as ET
from zipfile import ZipFile
from lxml import etree

def main(argv):
    inputdir = ''
    outputdir = ''
    extracted_files_dir_by_user = ''
    extracted_files_dir = ''
    fileformat = ''

    #read the arguments
    try:
       opts, args = getopt.getopt(argv,"hi:o:e:t:",["idir=","odir="])
    except getopt.GetoptError:
       print ('analyze.py -i <inputdir> -o <outputdir> -t <filetype>')
       sys.exit(2)

    for opt, arg in opts:
        if opt == '-h':
            print ('analyze.py -i <inputdir> -o <outputdir> -t <filetype>')
            sys.exit()
        elif opt == '-e':
            extracted_files_dir_by_user = arg
        elif opt in ("-i", "--idir"):
            inputdir = arg
        elif opt in ("-o", "--odir"):
            outputdir = arg
        elif opt == '-t':
            fileformat = arg

    if(extracted_files_dir_by_user == ''):
        # use default directory path for extracted ooxml files.
        extracted_files_dir = os.path.join(outputdir, 'extractedfiles')
        extract_files(inputdir, extracted_files_dir)
    else:
        # use user defined directory path for extracted ooxml files.
        extracted_files_dir = extracted_files_dir_by_user

    # create seperate result files for each ooxml document as <document name>.result in output directory
    # create seperate concanated texts for each ooxml document as <document name>.text in output directory
    for ext_dir in get_list_of_subdir(extracted_files_dir):
        i = ext_dir.rfind('/')
        sub_result_name = ext_dir[i+1:] + ".result"
        sub_texts_name = ext_dir[i+1:] + ".text"
        sub_result_list = []
        concatenated_texts_list = [] # holds concanated texts for each paragraph

        if fileformat == "pptx":
            count_pptx_elements(ext_dir, sub_result_list, concatenated_texts_list)
        elif fileformat == "xlsx":
            count_xlsx_elements(ext_dir, sub_result_list)
        else:
            print("File format is not supported")
            break

        sub_result_path = os.path.join(outputdir, sub_result_name)
        sub_texts_path = os.path.join(outputdir, sub_texts_name)

        # sort the result sub list according to tag names
        sub_result_list = sorted(sub_result_list, key=lambda x: list(x[0].keys())[0], reverse=False)
        concatenated_texts_list.sort()

        if os.path.exists(sub_result_path):
            os.remove(sub_result_path)
        if os.path.exists(sub_texts_path):
            os.remove(sub_texts_path)

        for i in sub_result_list:
            with open(sub_result_path, "a") as log_file:
                print(i, file=log_file)
            log_file.close()
        for i in concatenated_texts_list:
            with open(sub_texts_path, "a") as log_file:
                print(i, file=log_file)
            log_file.close()

    # no need to keep extracted files anymore.
    #if(os.path.exists(extracted_files_dir)):
    #    shutil.rmtree(extracted_files_dir)

# unzip all ooxml files into the given path
def extract_files(inputdir, extracted_files_dir):

    # clean extracted files directory firstly
    if(os.path.exists(extracted_files_dir)):
        shutil.rmtree(extracted_files_dir)

    # unzip files into the extracted files directory

    for filetype in get_list_of_subdir(inputdir):
        for filename in os.listdir(filetype):
            if (filename.endswith(".pptx") or       \
                filename.endswith(".docx") or       \
                filename.endswith(".xlsx")) and not \
                filename.startswith("~"):
                filepath = os.path.join(filetype, filename)
                extracted_file_path = os.path.join(extracted_files_dir, filename)

                try:
                    with ZipFile(filepath) as zipObj:
                        zipObj.extractall(extracted_file_path)
                except:
                    print("%s is problematic" % filename)
            else:
                continue

# get key of value in dictionary
def get_key(val, dict):
    for key, value in dict.items():
         if val == value:
             return str(key)
    return ''

# replace curlybrace namespaces with the shorten ones
def replace_namespace_with_alias(filename, element):
    namespaces = dict([node for _, node in ET.iterparse(filename, events=['start-ns'])])
    i = element.find('}')
    if i>=0:
        element_ns = element[1:i]
        element_ns_alias = get_key(element_ns, namespaces)
        if element_ns_alias !='':
            element = element.replace("{" + element_ns + "}", element_ns_alias + ":")
        else:
            element = element.replace("{" + element_ns + "}", "")
    return element

# decides which files should/shouldn't be analyzed.
def is_file_in_accepted_files(filename):
    if(filename.endswith(".xml") and ("ppt/slides/" in filename or "xl/worksheets" in filename)):
       return True

    return False

def read_shared_strings(shared_strings_list, shared_strings_path):
    tree = ET.parse(shared_strings_path)
    for child in tree.iter():
        if child.tag == '{http://schemas.openxmlformats.org/spreadsheetml/2006/main}t':
            shared_strings_list.append(child.text)

def get_pivot_table_range(sheet_relation_path):
    tree = ET.parse(sheet_relation_path)
    for elem in tree.iter():
        if elem.tag == "{http://schemas.openxmlformats.org/package/2006/relationships}Relationship" and\
           elem.attrib['Type'] == "http://schemas.openxmlformats.org/officeDocument/2006/relationships/pivotTable":
            i = sheet_relation_path.rfind('/')
            pivot_table_path = os.path.join(sheet_relation_path[:i], ".." ,elem.attrib['Target'])
            p_tree = ET.parse(pivot_table_path)
            for p_elem in p_tree.iter():
                if p_elem.tag == "{http://schemas.openxmlformats.org/spreadsheetml/2006/main}location" and \
                   p_elem.attrib['ref']:
                    return p_elem.attrib['ref']
    return ''

def is_cell_in_range(cell_id, cell_range):
    i = cell_range.find(':')
    start_range = cell_range[:i]
    end_range = cell_range[i+1:]

    if title_to_number(cell_id) > title_to_number(end_range) or\
       title_to_number(cell_id) < title_to_number(start_range):
       return False
    return True

def title_to_number(s):
    for i, c in enumerate(s):
        if c.isdigit():
            col = s[:i]
            row = s[i:]

            total = 0
            tmp = 0
            max_row = 2**20

            for j in range(len(col)-1, -1, -1):
                total += (ord(s[j])-64) * (26**tmp)
                tmp += 1

            result = (total-1)*max_row + int(row)
            return result

def count_xlsx_elements(extracted_files_dir, result_list):
    # make sure if extracted files directory not exist
    if not (os.path.exists(extracted_files_dir)):
        print("Extracted files directory is not exist")
        return

    list_of_files = get_list_of_files(extracted_files_dir)

    # read sharedString.xml and create a list
    tmp_id = -1
    shared_strings_list = []
    shared_strings_path = os.path.join(extracted_files_dir, 'xl/sharedStrings.xml')
    if os.path.exists(shared_strings_path):
        read_shared_strings(shared_strings_list, shared_strings_path)

    # parse xmls and count elements
    for xmlfile in list_of_files:
        if not is_file_in_accepted_files(xmlfile):
            continue

        print(xmlfile)
        sheetData_child_list = {"{http://schemas.openxmlformats.org/spreadsheetml/2006/main}sheetData",\
                                "{http://schemas.openxmlformats.org/spreadsheetml/2006/main}row",\
                                "{http://schemas.openxmlformats.org/spreadsheetml/2006/main}c",
                                "{http://schemas.openxmlformats.org/spreadsheetml/2006/main}v",
                                "{http://schemas.openxmlformats.org/spreadsheetml/2006/main}f"}
        i = xmlfile.rfind('/')
        rel_file_name = xmlfile[i+1:] + ".rels"
        sheet_relation_path = os.path.join(xmlfile[:i], "_rels", rel_file_name)
        pivot_range = ''
        if os.path.exists(sheet_relation_path):
            pivot_range = get_pivot_table_range(sheet_relation_path)
            print("Pivot range is: " + pivot_range)

        try:
            # start to count
            reset_cell = False
            for event, child in etree.iterparse(xmlfile, events=('start', 'end')):
                tag = child.tag #replace_namespace_with_alias(xmlfile, child.tag)
                text = child.text

                # handle sheetData
                if tag in sheetData_child_list:
                    if tag == '{http://schemas.openxmlformats.org/spreadsheetml/2006/main}c':
                        cell_id = child.get('r')
                        if pivot_range and is_cell_in_range(cell_id, pivot_range):
                            continue
                        cell_type = child.get('t')
                        if event == 'start':
                            # count tags
                            reset_cell = False
                            tmp_list = [{cell_id: 1},{},{},{}]
                            result_list.append(tmp_list)
                            tmp_id += 1
                        elif event == 'end':
                            reset_cell = True
                    elif tag == '{http://schemas.openxmlformats.org/spreadsheetml/2006/main}v':
                        if event == 'end' and reset_cell == False:
                            if cell_type == "s" and shared_strings_list[int(text)] not in (None, ''):
                                result_list[tmp_id][3][shared_strings_list[int(text)]] = 1
                            else:
                                result_list[tmp_id][3][text] = 1

        except Exception as exception:
            print("%s has %s " % (xmlfile, exception))

# counts tags, attribute names and values of xmls
def count_pptx_elements(extracted_files_dir, result_list, concanated_texts_list):

    # make sure if extracted files directory not exist
    if not (os.path.exists(extracted_files_dir)):
        print("Extracted files directory is not exist")
        return

    list_of_files = get_list_of_files(extracted_files_dir)

    # parse xmls and count elements
    for xmlfile in list_of_files:
        if not is_file_in_accepted_files(xmlfile):
            continue

        print(xmlfile)

        try:
            # start to count
            for event, child in etree.iterparse(xmlfile, events=('start', 'end')):
                tag = child.tag #replace_namespace_with_alias(xmlfile, child.tag)
                tag_idx = get_index_of_tag(tag, result_list)

                if event == "start":
                    # count tags
                    if (tag_idx == -1):
                        tmp_list = [{tag: 1},{},{},{}]
                        result_list.append(tmp_list)
                    else:
                        result_list[tag_idx][0][tag] += 1

                    #count attribute names and values of current tag
                    #for attr_name, attr_value in child.attrib.items():
                    #    attr_name = replace_namespace_with_alias(xmlfile, attr_name)
                    #    if not attr_name in result_list[tag_idx][1].keys():
                    #        result_list[tag_idx][1][attr_name] = 1
                    #    else:
                    #        result_list[tag_idx][1][attr_name] +=1

                    #    if not attr_value in result_list[tag_idx][2].keys():
                    #        result_list[tag_idx][2][attr_value] = 1
                    #    else:
                    #        result_list[tag_idx][2][attr_value] +=1

                    # concanated text will be resetted in every paragraph beginning
                    if tag == "{http://schemas.openxmlformats.org/drawingml/2006/main}p":
                        concatenated_text = ""

                if event == "end":
                    # Detect seperate texts in paragraph and concanate them.
                    if tag == "{http://schemas.openxmlformats.org/drawingml/2006/main}t" and str(child.text) != "None":
                        concatenated_text += str(child.text)
                    # End of the paragraph element, add the text as list item.
                    if tag == "{http://schemas.openxmlformats.org/drawingml/2006/main}p" and concatenated_text != "":
                        concanated_texts_list.append(concatenated_text)

                    # count text contents except consisted of whitespaces.
                    if not (str(child.text) == "None" or str(child.text).strip()==""):
                        if not child.text in result_list[tag_idx][3].keys():
                            result_list[tag_idx][3][child.text] = 1
                        else:
                            result_list[tag_idx][3][child.text] += 1

        except Exception as exception:
            print("%s has %s " % (xmlfile, exception))

# gets the position of "tag" element in result list. If element is not exist,
# return -1 that points the last index of the list.
def get_index_of_tag(tag, result_list):
    for idx, tag_list in enumerate(result_list):
        if tag in tag_list[0].keys():
            return idx
    return -1

# list all xmls in extracted files directory
def get_list_of_files(directory_name):

    list_of_file = os.listdir(directory_name)
    all_files = list()

    for filename in list_of_file:
        full_path = os.path.join(directory_name, filename)
        if os.path.isdir(full_path):
            all_files = all_files + get_list_of_files(full_path)
        else:
            all_files.append(full_path)

    return all_files

def get_list_of_subdir(directory_name):

    list_of_file = os.listdir(directory_name)
    subdirs = list()

    for filename in list_of_file:
        full_path = os.path.join(directory_name, filename)
        if os.path.isdir(full_path):
            subdirs.append(full_path)

    return subdirs

if __name__ == "__main__":
    main(sys.argv[1:])
