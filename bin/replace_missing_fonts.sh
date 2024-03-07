#!/bin/bash

declare -A replacements
replacements["Agency FB"]="Noto Sans"
replacements["Segoe UI"]="Noto Sans"
replacements["Univers 45 Light"]="Noto Sans"
replacements["Trebuchet MS"]="Noto Sans"
replacements["Perpetua"]="Noto Sans"
replacements["Calibri Light"]="Noto Sans"
replacements["Rockwell"]="Noto Sans"
replacements["DFKai-SB"]="Noto Sans"
replacements["Gill Sans MT"]="Noto Sans"
replacements["BentonSans Medium"]="Noto Sans"
replacements["BentonSans"]="Noto Sans"
replacements["AdvPS88D1"]="Noto Sans"
replacements["NexusSansOT"]="Noto Sans"
replacements["Segoe Script"]="Noto Sans"
replacements["Impact"]="Noto Sans"
replacements["Century Gothic"]="Noto Sans"
replacements["Kings Caslon Text"]="Noto Sans"
replacements["Times"]="Liberation Serif"
replacements["Jokerman"]="Noto Sans"
replacements["FreeSans"]="Noto Sans"
replacements["DINPro-Medium"]="Noto Sans"
replacements["Open Sans Light"]="Noto Sans"
replacements["Lora - regular"]="Noto Sans"

extracted_folder=".temp_extracted"

for file in $(find "$1" -type f); do
    file_name=$(basename "$file")
    current_extension="${file_name##*.}"

    if [[ $current_extension == "docx" || $current_extension == "xlsx" || $current_extension == "pptx" || $current_extension == "odt" ]]; then
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
                # Replace only if it's between quotes
                if grep -q "\"$key\"" "$subfile"; then
                    sed -i "s/\"$key\"/\"$value\"/g" "$subfile"
                    file_changed=true
                # or between '&quot;'
                elif grep -q "&quot;$key&quot;" "$subfile"; then
                    sed -i "s/&quot;$key&quot;/\&quot;$value\&quot;/g" "$subfile"
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

