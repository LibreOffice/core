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



#ifndef _LAYOUT_PRE_HXX
#define _LAYOUT_PRE_HXX

#if ENABLE_LAYOUT

///* Allow re-inclusion for next hxx file. */
#undef _LAYOUT_POST_HXX

#define AdvancedButton layout::AdvancedButton
#define ApplyButton layout::ApplyButton
#define Box layout::Box
#define Button layout::Button
#define CancelButton layout::CancelButton
#define CheckBox layout::CheckBox
#define ComboBox layout::ComboBox
#define Container layout::Container
#define Control layout::Control
#define Dialog layout::Dialog
#define Edit layout::Edit
#define ErrorBox layout::ErrorBox
#define FixedImage layout::FixedImage
#define FixedInfo layout::FixedInfo
#define FixedLine layout::FixedLine
#define FixedText layout::FixedText
#define HBox layout::HBox
#define HelpButton layout::HelpButton
#define IgnoreButton layout::IgnoreButton
#define ImageButton layout::ImageButton
#define InfoBox layout::InfoBox
#define LocalizedString layout::LocalizedString
#define ListBox layout::ListBox
#define MessBox layout::MessBox
#define MessageBox layout::MessageBox
#define MetricField layout::MetricField
#define MetricFormatter layout::MetricFormatter
#define MoreButton layout::MoreButton
#define MultiLineEdit layout::MultiLineEdit
#define MultiListBox layout::MultiListBox
#define NoButton layout::NoButton
#define NumericField layout::NumericField
#define NumericFormatter layout::NumericFormatter
#define OKButton layout::OKButton
#define Plugin layout::Plugin
#define ProgressBar layout::ProgressBar
#define PushButton layout::PushButton
#define QueryBox layout::QueryBox
#define RadioButton layout::RadioButton
#define ResetButton layout::ResetButton
#define RetryButton layout::RetryButton
#define SfxTabDialog layout::SfxTabDialog
#define SfxTabPage layout::SfxTabPage
#if ENABLE_LAYOUT
#define SvxFontListBox layout::SvxFontListBox
#define SvxLanguageBox layout::SvxLanguageBox
#endif
#define SpinField layout::SpinField
#define TabControl layout::TabControl
#define TabPage layout::TabPage
#define Table layout::Table
#define VBox layout::VBox
#define WarningBox layout::WarningBox
#define YesButton layout::YesButton

#define ModalDialog Dialog
#define ModelessDialog Dialog
#define ScExpandedFixedText FixedText
#define SfxDialog Dialog
#define SfxModalDialog Dialog
#define SfxModelessDialog Dialog
#define TabDialog Dialog

#define Window ::Window

#else

#define LocalizedString String

#endif /* ENABLE_LAYOUT */

#endif /* _LAYOUT_PRE_HXX */
