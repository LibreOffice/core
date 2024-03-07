#!/bin/bash

declare -A replacements
replacements["Agency FB"]="Noto Sans"
replacements["Noto Sans Light"]="Noto Sans"
replacements["Segoe UI"]="Noto Sans"
replacements["Univers 45 Light"]="Noto Sans"
replacements["Trebuchet MS"]="Noto Sans"
replacements["Georgia"]="Noto Serif"
replacements["Perpetua"]="Noto Sans"
replacements["Garamond"]="Noto Serif"
replacements["Calibri Light"]="Noto Sans"
replacements["Consolas"]="DejaVu Sans Mono"
replacements["Verdana"]="Noto Sans"
replacements["Rockwell"]="Noto Sans"
replacements["Tms Rmn"]="DejaVu Sans"
replacements["Tahoma"]="Noto Sans"
replacements["DFKai-SB"]="Noto Sans"
replacements["Gill Sans MT"]="Noto Sans"
replacements["Helvetica"]="Liberation Sans"
replacements["Liberation Serif"]="DejaVu Sans"
replacements["BentonSans Medium"]="Noto Sans"
replacements["BentonSans"]="Noto Sans"
replacements["AdvPS88D1"]="Noto Sans"
replacements["NexusSansOT"]="Noto Sans"
replacements["Segoe Script"]="Noto Sans"

extracted_folder=".temp_extracted"

for file in $(find "$1" -type f); do
    file_name=$(basename "$file")
    current_extension="${file_name##*.}"

    if [[ $current_extension == "docx" || $current_extension == "xlsx" || $current_extension == "pptx" ]]; then
        base_name="${file_name%.*}"

        # move the file to a new .zip file
        cp "$file" "${base_name}.zip"

        # extract the zip file to a temporary folder
        unzip -qq ./"${base_name}.zip"  -d "$extracted_folder" > /dev/null

        for key in "${!replacements[@]}"
        do
            file_changed=false
            value=${replacements[$key]}
            for subfile in $(find "$extracted_folder" -type f); do
                if grep -q "$key" "$subfile"; then
                    # use sed to replace the string in the file if it exists
                    sed -i "s/$key/$value/g" "$subfile"
                    file_changed=true
                fi
            done

            if [ "$file_changed" = true ]; then
                # Create a new zip file with the modified files
                cd "$extracted_folder"; zip -r ../"${base_name}.zip" . > /dev/null; cd ..

                mv "${base_name}.zip" "$file"

                echo "Replacing '$key' with '$value' in $file"
            fi
        done

        # Clean up the temporary extracted folder
        rm -rf "$extracted_folder"
    fi
done

