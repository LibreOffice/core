#!/usr/bin/python

import sys, getopt, os, shutil, pprint
import xml.etree.ElementTree as ET
from zipfile import ZipFile

def main(argv):
    inputdir = ''
    outputdir = ''
    extracted_files_dir_by_user = ''

    #read the arguments
    try:
       opts, args = getopt.getopt(argv,"hi:o:e:",["idir=","odir="])
    except getopt.GetoptError:
       print ('analyze.py -i <inputdir> -o <outputdir>')
       sys.exit(2)

    for opt, arg in opts:
       if opt == '-h':
          print ('analyze.py -i <inputdir> -o <outputdir>')
          sys.exit()
       elif opt == '-e':
          extracted_files_dir_by_user = arg
       elif opt in ("-i", "--idir"):
          inputdir = arg
       elif opt in ("-o", "--odir"):
          outputdir = arg

    # holds the result structer of analyze
    result_list = []

    if(extracted_files_dir_by_user == ''):
        # use default directory path for extracted ooxml files.
        extracted_files_dir = os.path.join(outputdir, 'extractedfiles')

        extract_files(inputdir, extracted_files_dir)
        count_elements(extracted_files_dir, result_list)
    else:
        # use user defined directory path for extracted ooxml files.
        count_elements(extracted_files_dir_by_user, result_list)

    pprint.pprint(result_list)

# unzip all ooxml files into the given path
def extract_files(inputdir, extracted_files_dir):

    # clean extracted files directory firstly
    if(os.path.exists(extracted_files_dir)):
        shutil.rmtree(extracted_files_dir)

    # holds directory names for each ooxml document in extracted files dir.
    counter = 1

    # unzip files into the extracted files directory
    for filename in os.listdir(inputdir):
        if (filename.endswith(".pptx") or       \
            filename.endswith(".docx") or       \
            filename.endswith(".xlsx")) and not \
            filename.startswith("~"):

            filepath = os.path.join(inputdir, filename)
            extracted_file_path = os.path.join(extracted_files_dir, str(counter))

            with ZipFile(filepath) as zipObj:
                zipObj.extractall(extracted_file_path)

            counter += 1
        else:
            continue

# counts tags, attribute names and values of xmls
def count_elements(extracted_files_dir, result_list):

    # make sure if extracted files directory exist
    if not (os.path.exists(extracted_files_dir)):
        print("Extracted files directory is not exist")
        return

    list_of_files = get_list_of_files(extracted_files_dir)

    # parse xmls and count elements
    for xmlfile in list_of_files:
        if(xmlfile.endswith(".xml")):
            tree = ET.parse(xmlfile)
            root = tree.getroot()

            # start to count
            for child in root.iter():
                tag = str(child.tag)
                tag_idx = get_index_of_tag(tag, result_list)

                # count tags
                if (tag_idx == -1):
                    tmp_list = [{tag: 1},{},{},{}]
                    result_list.append(tmp_list)
                else:
                    result_list[tag_idx][0][tag] += 1

                # count attribute names and values of current tag
                for attr_name, attr_value in child.attrib.items():
                    if not attr_name in result_list[tag_idx][1].keys():
                        result_list[tag_idx][1][attr_name] = 1
                    else:
                        result_list[tag_idx][1][attr_name] +=1

                    if not attr_value in result_list[tag_idx][2].keys():
                        result_list[tag_idx][2][attr_value] = 1
                    else:
                        result_list[tag_idx][2][attr_value] +=1

                if not (str(child.text) == "None"):
                    if not child.text in result_list[tag_idx][3].keys():
                        result_list[tag_idx][3][child.text] = 1
                    else:
                        result_list[tag_idx][3][child.text] += 1

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

if __name__ == "__main__":
    main(sys.argv[1:])
